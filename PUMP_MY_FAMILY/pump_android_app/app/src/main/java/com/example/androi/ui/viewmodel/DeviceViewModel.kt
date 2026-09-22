package com.example.androi.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.androi.data.model.*
import com.example.androi.data.repository.PumpRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

data class DeviceScreenState(
    val devices: List<DeviceDto> = emptyList(),
    val currentDevice: DeviceDto? = null,
    val deviceUiState: DeviceUiState = DeviceUiState(),
    val logs: List<PumpLogDto> = emptyList(),
    val notifications: List<AppNotification> = listOf(
        AppNotification(
            title = "Hệ Thống Sẵn Sàng",
            message = "Đã kết nối trực tiếp với ESP32-S3 Master và Cloud Server 180.93.113.40",
            type = NotificationType.INFO
        )
    ),
    val activeBanner: AppNotification? = null,
    val isLoading: Boolean = false,
    val isActionInProgress: Boolean = false,
    val errorMessage: String? = null,
    val userNotice: String? = null
)

class DeviceViewModel(private val repository: PumpRepository = PumpRepository()) : ViewModel() {

    private val _state = MutableStateFlow(DeviceScreenState())
    val state = _state.asStateFlow()

    val deviceUiState = _state.map { it.deviceUiState }.stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        DeviceUiState()
    )

    private var pollingJob: Job? = null
    private var bannerDismissJob: Job? = null
    private var lastWaterState: String = "NORMAL" // "FULL", "LOW", "NORMAL"

    init {
        loadDevices()
    }

    /**
     * Bắn thông báo lên đỉnh màn hình trượt xuống và tự thu lại sau 3 giây
     * Đồng thời lưu vào danh sách Hộp Thư trong icon Cái Chuông
     */
    fun showTopNotification(notification: AppNotification) {
        _state.update {
            it.copy(
                notifications = listOf(notification) + it.notifications,
                activeBanner = notification
            )
        }
        bannerDismissJob?.cancel()
        bannerDismissJob = viewModelScope.launch {
            delay(3000) // 3 giây tự động trượt lên ẩn đi
            _state.update { it.copy(activeBanner = null) }
        }
    }

    fun dismissBanner() {
        bannerDismissJob?.cancel()
        _state.update { it.copy(activeBanner = null) }
    }

    fun clearNotifications() {
        _state.update { it.copy(notifications = emptyList()) }
    }

    /**
     * Tự động kiểm tra cảnh báo Nước Đầy và Nước Cạn theo ngưỡng
     * Bỏ qua khi Node Bể mất kết nối để tránh hiểu nhầm chạy khô / nước không tăng
     */
    private fun checkWaterThresholds(waterPercent: Float, minPct: Int, maxPct: Int, isTankOnline: Boolean = true) {
        if (!isTankOnline || waterPercent <= 0f) return

        if (waterPercent >= maxPct && lastWaterState != "FULL") {
            lastWaterState = "FULL"
            showTopNotification(
                AppNotification(
                    title = "Bể Nước Đã Đầy",
                    message = "Mực nước bể đạt ${"%.1f".format(waterPercent)}%. Máy bơm tự động ngắt để chống tràn bể!",
                    type = NotificationType.WATER_FULL
                )
            )
        } else if (waterPercent <= minPct && lastWaterState != "LOW") {
            lastWaterState = "LOW"
            showTopNotification(
                AppNotification(
                    title = "Cảnh Báo Nước Cạn",
                    message = "Mực nước bể còn ${"%.1f".format(waterPercent)}%, dưới ngưỡng an toàn! Bơm đang bật để cấp nước.",
                    type = NotificationType.WATER_LOW
                )
            )
        } else if (waterPercent > (minPct + 5) && waterPercent < (maxPct - 5)) {
            lastWaterState = "NORMAL"
        }
    }

    /**
     * Tải danh sách máy bơm từ Server (Đồng bộ với Web Dashboard)
     */
    fun loadDevices() {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true, errorMessage = null) }
            val result = repository.getDevices()
            result.onSuccess { response ->
                val all = response.allDevices
                val current = if (all.isNotEmpty()) {
                    val existingId = _state.value.currentDevice?.id
                    all.find { it.id == existingId } ?: all.first()
                } else null

                val ui = current?.toUiState() ?: DeviceUiState(
                    id = "",
                    deviceCode = "",
                    name = "Chưa có máy bơm nào",
                    isOnline = false,
                    isPumpRunning = false,
                    waterPercent = 0f
                )
                _state.update {
                    it.copy(
                        devices = all,
                        currentDevice = current,
                        deviceUiState = ui,
                        isLoading = false
                    )
                }

                if (current != null) {
                    checkWaterThresholds(ui.waterPercent, ui.minWaterPercent, ui.maxWaterPercent, ui.isTankOnline)
                    loadLogs(current.id)
                }
                startPolling()
            }.onFailure { error ->
                _state.update { it.copy(isLoading = false, errorMessage = error.message) }
            }
        }
    }

    /**
     * Bắt đầu Polling 2.5s tương tự `setInterval(fetchDeviceData, 2500)` trên Web
     */
    fun startPolling() {
        if (pollingJob?.isActive == true) return
        pollingJob = viewModelScope.launch {
            while (isActive) {
                delay(2500)
                fetchDevicesQuietly()
            }
        }
    }

    fun stopPolling() {
        pollingJob?.cancel()
        pollingJob = null
    }

    private suspend fun fetchDevicesQuietly() {
        repository.getDevices().onSuccess { response ->
            val all = response.allDevices
            val currentId = _state.value.currentDevice?.id
            val updatedCurrent = if (currentId != null) all.find { it.id == currentId } else all.firstOrNull()

            val ui = updatedCurrent?.toUiState() ?: if (all.isEmpty()) {
                DeviceUiState(
                    id = "",
                    deviceCode = "",
                    name = "Chưa có máy bơm nào",
                    isOnline = false,
                    isPumpRunning = false,
                    waterPercent = 0f
                )
            } else _state.value.deviceUiState
            _state.update { prev ->
                prev.copy(
                    devices = all,
                    currentDevice = updatedCurrent,
                    deviceUiState = ui
                )
            }

            if (updatedCurrent != null) {
                checkWaterThresholds(ui.waterPercent, ui.minWaterPercent, ui.maxWaterPercent, ui.isTankOnline)
                // Đồng bộ nhật ký sự kiện realtime liên tục từ Cloud
                repository.getPumpLogs(updatedCurrent.id, 20).onSuccess { logsList ->
                    _state.update { it.copy(logs = logsList) }
                }
            }
        }
    }

    fun selectDevice(deviceId: String) {
        val device = _state.value.devices.find { it.id == deviceId } ?: return
        val ui = device.toUiState()
        _state.update {
            it.copy(
                currentDevice = device,
                deviceUiState = ui
            )
        }
        checkWaterThresholds(ui.waterPercent, ui.minWaterPercent, ui.maxWaterPercent, ui.isTankOnline)
        loadLogs(deviceId)
    }

    /**
     * Điều khiển máy Bơm: PUMP_ON / PUMP_OFF
     */
     fun togglePump(targetState: Boolean) {
        val device = _state.value.currentDevice ?: _state.value.devices.firstOrNull() ?: return

        // Nếu máy đang ở chế độ TỰ ĐỘNG, chặn không cho bật/tắt thủ công và hiện thông báo
        if (device.isAutoMode) {
            showTopNotification(
                AppNotification(
                    title = "Đang Ở Chế Độ Tự Động",
                    message = "Máy bơm đang chạy Tự Động! Vui lòng vào chi tiết chuyển sang Thủ Công để điều khiển.",
                    type = NotificationType.INFO
                )
            )
            return
        }

        val action = if (targetState) "PUMP_ON" else "PUMP_OFF"

        // Bắn banner thông báo trượt từ trên xuống 3s
        showTopNotification(
            AppNotification(
                title = if (targetState) "Đã Bật Máy Bơm" else "Đã Tắt Máy Bơm",
                message = "Đã gửi lệnh [$action] thành công tới ${device.name.ifBlank { "máy bơm" }}!",
                type = if (targetState) NotificationType.PUMP_ON else NotificationType.PUMP_OFF
            )
        )

        // Cập nhật giao diện tức thì (Optimistic UI update)
        _state.update {
            it.copy(
                currentDevice = device,
                deviceUiState = it.deviceUiState.copy(isPumpRunning = targetState),
                isActionInProgress = true
            )
        }

        viewModelScope.launch {
            val result = repository.controlPump(device.id, action)
            result.onSuccess { msg ->
                _state.update { it.copy(isActionInProgress = false) }
                delay(1000)
                fetchDevicesQuietly()
                loadLogs(device.id)
            }.onFailure { err ->
                // Khôi phục lại trạng thái cũ nếu lỗi
                _state.update {
                    it.copy(
                        deviceUiState = it.deviceUiState.copy(isPumpRunning = !targetState),
                        isActionInProgress = false
                    )
                }
                showTopNotification(
                    AppNotification(
                        title = "Lệnh Thất Bại",
                        message = err.message ?: "Không thể điều khiển máy bơm",
                        type = NotificationType.WARNING
                    )
                )
            }
        }
    }

    /**
     * Chuyển chế độ Tự Động (AUTO_MODE) / Thủ Công (MANUAL_MODE)
     */
    fun toggleAutoMode(isAuto: Boolean) {
        val device = _state.value.currentDevice ?: _state.value.devices.firstOrNull() ?: return
        val action = if (isAuto) "AUTO_MODE" else "MANUAL_MODE"

        showTopNotification(
            AppNotification(
                title = if (isAuto) "Chế Độ Tự Động" else "Chế Độ Thủ Công",
                message = if (isAuto) "Hệ thống sẽ tự động bật khi cạn và ngắt khi đầy!" else "Đã chuyển sang chế độ bật tắt thủ công.",
                type = NotificationType.INFO
            )
        )

        _state.update {
            it.copy(
                currentDevice = device,
                deviceUiState = it.deviceUiState.copy(isAutoMode = isAuto)
            )
        }

        viewModelScope.launch {
            repository.controlPump(device.id, action).onSuccess { msg ->
                delay(1000)
                fetchDevicesQuietly()
            }.onFailure { err ->
                _state.update {
                    it.copy(
                        deviceUiState = it.deviceUiState.copy(isAutoMode = !isAuto)
                    )
                }
                showTopNotification(
                    AppNotification(
                        title = "Lỗi Chuyển Chế Độ",
                        message = err.message ?: "Không thể đổi chế độ vận hành",
                        type = NotificationType.WARNING
                    )
                )
            }
        }
    }

    /**
     * Khóa trẻ em (CHILD_LOCK_ON / CHILD_LOCK_OFF)
     */
    fun toggleChildLock(isLock: Boolean) {
        val device = _state.value.currentDevice ?: _state.value.devices.firstOrNull() ?: return
        val action = if (isLock) "CHILD_LOCK_ON" else "CHILD_LOCK_OFF"

        showTopNotification(
            AppNotification(
                title = if (isLock) "Đã Khóa Trẻ Em" else "Đã Mở Khóa Trẻ Em",
                message = if (isLock) "Nút bấm cơ tại tủ điện ESP32 ngoài trời đã bị vô hiệu hóa an toàn." else "Đã mở khóa các nút bấm vật lý ngoài sân.",
                type = NotificationType.INFO
            )
        )

        _state.update {
            it.copy(
                currentDevice = device,
                deviceUiState = it.deviceUiState.copy(isChildLock = isLock)
            )
        }

        viewModelScope.launch {
            repository.controlPump(device.id, action).onSuccess { msg ->
                delay(1000)
                fetchDevicesQuietly()
            }.onFailure { err ->
                _state.update {
                    it.copy(
                        deviceUiState = it.deviceUiState.copy(isChildLock = !isLock)
                    )
                }
                showTopNotification(
                    AppNotification(
                        title = "Lỗi Khóa Nút Bấm",
                        message = err.message ?: "Không thể thao tác Khóa trẻ em",
                        type = NotificationType.WARNING
                    )
                )
            }
        }
    }

    /**
     * Cấu hình téc nước & ngưỡng tự động (gửi xuống ESP32 qua MQTT)
     */
    fun saveTankConfig(tankHeight: Float, offset: Float, minPct: Int, maxPct: Int) {
        val device = _state.value.currentDevice ?: _state.value.devices.firstOrNull() ?: return
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true, errorMessage = null) }
            val res = repository.updateTankConfig(device.id, tankHeight, offset, minPct, maxPct)
            res.onSuccess { msg ->
                _state.update {
                    it.copy(
                        isLoading = false,
                        deviceUiState = it.deviceUiState.copy(
                            tankHeightCm = tankHeight,
                            sensorOffsetCm = offset,
                            minWaterPercent = minPct,
                            maxWaterPercent = maxPct
                        )
                    )
                }
                showTopNotification(
                    AppNotification(
                        title = "Cấu Hình Thành Công",
                        message = "Đã lưu cài đặt téc: Cao ${tankHeight}cm, Bật: $minPct%, Tắt: $maxPct%",
                        type = NotificationType.INFO
                    )
                )
                delay(1000)
                fetchDevicesQuietly()
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                showTopNotification(
                    AppNotification(
                        title = "Lưu Cấu Hình Thất Bại",
                        message = err.message ?: "Lỗi máy chủ",
                        type = NotificationType.WARNING
                    )
                )
            }
        }
    }

    /**
     * Lấy danh sách lịch sử bật tắt bơm
     */
    fun loadLogs(deviceId: String? = null) {
        val id = deviceId ?: _state.value.currentDevice?.id ?: _state.value.devices.firstOrNull()?.id ?: return
        viewModelScope.launch {
            repository.getPumpLogs(id).onSuccess { logsList ->
                _state.update { it.copy(logs = logsList) }
            }
        }
    }

    /**
     * Thêm máy bơm mới
     */
    fun createDevice(code: String, name: String, onComplete: (Boolean) -> Unit = {}) {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.createDevice(code, name)
            res.onSuccess {
                _state.update { it.copy(isLoading = false) }
                showTopNotification(
                    AppNotification(
                        title = "Thêm Thiết Bị Mới",
                        message = "Đã thêm máy bơm [$name] thành công!",
                        type = NotificationType.INFO
                    )
                )
                loadDevices()
                onComplete(true)
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                loadDevices()
                onComplete(false)
            }
        }
    }

    /**
     * Chia sẻ quyền máy bơm cho người khác
     */
    fun shareDevice(targetEmail: String, permission: String, onComplete: (Boolean) -> Unit = {}) {
        val device = _state.value.currentDevice ?: _state.value.devices.firstOrNull() ?: return
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.shareDevice(device.id, targetEmail, permission)
            res.onSuccess { msg ->
                _state.update { it.copy(isLoading = false) }
                showTopNotification(
                    AppNotification(
                        title = "Đã Chia Sẻ Thiết Bị",
                        message = "Đã cấp quyền $permission cho $targetEmail thành công!",
                        type = NotificationType.INFO
                    )
                )
                onComplete(true)
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                showTopNotification(
                    AppNotification(
                        title = "Chia Sẻ Thất Bại",
                        message = err.message ?: "Không thể chia sẻ thiết bị",
                        type = NotificationType.WARNING
                    )
                )
                onComplete(false)
            }
        }
    }

    /**
     * Xóa máy bơm khỏi tài khoản
     */
    fun deleteDevice(deviceId: String, onComplete: (Boolean) -> Unit = {}) {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.deleteDevice(deviceId)
            res.onSuccess {
                // Cập nhật danh sách thiết bị ngay lập tức
                _state.update { prev ->
                    val remaining = prev.devices.filter { it.id != deviceId }
                    val nextDev = remaining.firstOrNull()
                    prev.copy(
                        isLoading = false,
                        devices = remaining,
                        currentDevice = nextDev,
                        deviceUiState = nextDev?.toUiState() ?: DeviceUiState(
                            id = "",
                            deviceCode = "",
                            name = "Chưa có máy bơm nào",
                            isOnline = false,
                            isPumpRunning = false,
                            waterPercent = 0f
                        )
                    )
                }

                showTopNotification(
                    AppNotification(
                        title = "Đã Xóa Thiết Bị",
                        message = "Đã xóa máy bơm khỏi tài khoản của bạn!",
                        type = NotificationType.INFO
                    )
                )
                loadDevices()
                onComplete(true)
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                showTopNotification(
                    AppNotification(
                        title = "Xóa Thất Bại",
                        message = err.message ?: "Không thể xóa máy bơm",
                        type = NotificationType.WARNING
                    )
                )
                onComplete(false)
            }
        }
    }

    fun clearNotice() {
        _state.update { it.copy(userNotice = null, errorMessage = null) }
    }

    override fun onCleared() {
        super.onCleared()
        stopPolling()
    }
}
