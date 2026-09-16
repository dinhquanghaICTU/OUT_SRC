package com.example.androi.data.model

import com.google.gson.annotations.SerializedName

/**
 * Trạng thái của thiết bị máy bơm & bể nước (hiển thị trên UI)
 */
data class DeviceUiState(
    val id: String = "",
    val deviceCode: String = "PUMP_ESP32S3_01",
    val name: String = "Máy Bơm Gia Đình",
    val isOnline: Boolean = true,
    val isTankOnline: Boolean = true,
    val isPumpRunning: Boolean = false,
    val isAutoMode: Boolean = true,
    val isChildLock: Boolean = false,
    val waterPercent: Float = 89.6f,
    val distanceCm: Float = 37.5f,
    val batteryVoltage: Float = 4.15f,
    val pumpRuntimeSec: Int = 0,
    val tankHeightCm: Float = 150f,
    val sensorOffsetCm: Float = 20f,
    val minWaterPercent: Int = 30,
    val maxWaterPercent: Int = 95,
    val firmwareVersion: String = "1.0.4",
    val tankFirmwareVersion: String = "1.0.4",
    val role: String = "OWNER",          // "OWNER" hoặc "MEMBER"
    val permission: String = "CAN_CONTROL" // "CAN_CONTROL" hoặc "VIEW_ONLY"
)

/**
 * Thẻ thiết bị hiển thị trong danh sách / lưới 2x2
 */
data class DeviceCardItem(
    val id: String,
    val title: String,
    val subtitle: String,
    val iconType: DeviceIconType,
    val isRunning: Boolean,
    val statusText: String,
    val isSwitchEnabled: Boolean = true
)

enum class DeviceIconType {
    PUMP,
    WATER_TANK,
    CHILD_LOCK,
    SOLAR_BATTERY,
    LIGHT,
    AC
}

/**
 * Loại thông báo hệ thống
 */
enum class NotificationType {
    WATER_FULL,  // Nước đầy téc (tự ngắt hoặc cảnh báo)
    WATER_LOW,   // Nước cạn bể (tự bật hoặc cảnh báo)
    PUMP_ON,     // Bật máy bơm
    PUMP_OFF,    // Tắt máy bơm
    WARNING,     // Cảnh báo chung (mất kết nối, pin yếu)
    INFO         // Thông tin hệ thống
}

/**
 * Đối tượng thông báo hiển thị trong Hộp Thư / Chuông thông báo
 */
data class AppNotification(
    val id: String = java.util.UUID.randomUUID().toString(),
    val title: String,
    val message: String,
    val type: NotificationType = NotificationType.INFO,
    val timestamp: Long = System.currentTimeMillis(),
    val isRead: Boolean = false
)

// ==================== DTO CHO API BACKEND ====================

data class DeviceDto(
    val id: String = "",
    @SerializedName("device_code") val deviceCode: String = "",
    val name: String = "",
    @SerializedName("is_online") val isOnline: Boolean = false,
    @SerializedName("is_tank_online") val isTankOnline: Boolean? = true,
    @SerializedName("is_pump_running") val isPumpRunning: Boolean = false,
    @SerializedName("is_auto_mode") val isAutoMode: Boolean = true,
    @SerializedName("is_child_lock") val isChildLock: Boolean = false,
    @SerializedName("water_level") val waterLevel: Float? = 0f,
    @SerializedName("tank_height_cm") val tankHeightCm: Float? = 150f,
    @SerializedName("sensor_offset_cm") val sensorOffsetCm: Float? = 20f,
    @SerializedName("min_water_percent") val minWaterPercent: Int? = 30,
    @SerializedName("max_water_percent") val maxWaterPercent: Int? = 95,
    @SerializedName("firmware_version") val firmwareVersion: String? = "1.0.0",
    @SerializedName("tank_firmware_version") val tankFirmwareVersion: String? = "1.0.0",
    @SerializedName("battery_voltage") val batteryVoltage: Float? = 0f,
    @SerializedName("distance_cm") val distanceCm: Float? = 0f,
    @SerializedName("pump_runtime") val pumpRuntime: Int? = 0,
    val role: String? = "OWNER",
    val permission: String? = "CAN_CONTROL"
) {
    fun toUiState(): DeviceUiState {
        return DeviceUiState(
            id = id,
            deviceCode = deviceCode,
            name = name.ifBlank { "Máy Bơm Gia Đình" },
            isOnline = isOnline,
            isTankOnline = isTankOnline ?: true,
            isPumpRunning = isPumpRunning,
            isAutoMode = isAutoMode,
            isChildLock = isChildLock,
            waterPercent = (waterLevel ?: 0f).coerceIn(0f, 100f),
            distanceCm = distanceCm ?: 0f,
            batteryVoltage = batteryVoltage ?: 0f,
            pumpRuntimeSec = pumpRuntime ?: 0,
            tankHeightCm = tankHeightCm ?: 150f,
            sensorOffsetCm = sensorOffsetCm ?: 20f,
            minWaterPercent = minWaterPercent ?: 30,
            maxWaterPercent = maxWaterPercent ?: 95,
            firmwareVersion = firmwareVersion ?: "1.0.0",
            tankFirmwareVersion = tankFirmwareVersion ?: "1.0.0",
            role = role ?: "OWNER",
            permission = permission ?: "CAN_CONTROL"
        )
    }
}

data class DevicesResponse(
    @SerializedName("owned_devices") val ownedDevices: List<DeviceDto> = emptyList(),
    @SerializedName("shared_devices") val sharedDevices: List<DeviceDto> = emptyList()
) {
    val allDevices: List<DeviceDto>
        get() = ownedDevices + sharedDevices
}

data class PumpControlRequest(
    val action: String
)

data class TankConfigRequest(
    @SerializedName("tank_height") val tankHeight: Float,
    val offset: Float,
    @SerializedName("min_pct") val minPct: Int,
    @SerializedName("max_pct") val maxPct: Int
)

data class PumpLogDto(
    val id: String = "",
    val action: String = "",
    @SerializedName("water_level") val waterLevel: Float? = null,
    @SerializedName("created_at") val createdAt: String = "",
    @SerializedName("triggered_by") val triggeredBy: Any? = null
)

data class PumpLogsResponse(
    val logs: List<PumpLogDto> = emptyList()
)

data class DeviceCreateRequest(
    @SerializedName("device_code") val deviceCode: String,
    val name: String
)

data class DeviceShareRequest(
    @SerializedName("target_email") val targetEmail: String,
    val permission: String = "CAN_CONTROL" // "CAN_CONTROL" hoặc "VIEW_ONLY"
)

data class ApiResponse(
    val message: String? = null,
    val status: String? = null
)

// ==================== ADMIN MODELS ====================

data class AdminUserDto(
    val id: String = "",
    val email: String = "",
    @SerializedName("full_name") val fullName: String = "",
    val role: String = "USER",
    @SerializedName("owned_devices_count") val ownedDevicesCount: Int = 0,
    @SerializedName("created_at") val createdAt: String = ""
)

data class AdminUserCreateRequest(
    val email: String,
    val password: String,
    @SerializedName("full_name") val fullName: String,
    val role: String = "USER"
)

data class AdminUserUpdateRequest(
    @SerializedName("full_name") val fullName: String? = null,
    val role: String? = null,
    val password: String? = null
)

data class OtaTriggerRequest(
    val target: String, // "esp32s3_cabinet" hoặc "esp32_tank"
    val version: String,
    val url: String,
    val size: Long,
    val filename: String,
    val md5: String? = null
)

data class OtaProgressResponse(
    val target: String? = null,
    val status: String? = null,
    val percent: Int? = 0,
    val bytes: Long? = 0,
    val total: Long? = 0,
    val version: String? = null,
    val message: String? = null,
    val error: String? = null
)
