package com.example.androi.ui.navigation

import android.widget.Toast
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import com.example.androi.ui.components.common.FloatingBottomBar
import com.example.androi.ui.components.common.NavigationTab
import com.example.androi.ui.screens.admin.AdminDashboardScreen
import com.example.androi.ui.screens.detail.DetailControlScreen
import com.example.androi.ui.screens.devices.RoomGridScreen
import com.example.androi.ui.screens.overview.OverviewScreen
import com.example.androi.ui.viewmodel.DeviceViewModel

/**
 * Container chính bao bọc toàn bộ ứng dụng:
 * - Điều hướng mượt mà: Trang chủ -> Chi tiết bể/bơm -> Danh sách thiết bị -> Quản trị Admin
 * - Hiển thị FloatingBottomBar màu đen nổi ở đáy màn hình
 * - Tự động kết nối DeviceViewModel và Polling 2.5s từ server 180.93.113.40
 */
@Composable
fun MainAppContainer(
    deviceViewModel: DeviceViewModel = remember { DeviceViewModel() },
    onLogout: () -> Unit = {}
) {
    val context = LocalContext.current
    var currentTab by remember { mutableStateOf(NavigationTab.HOME) }
    var activeScreen by remember { mutableStateOf("OVERVIEW") }
    var showNotificationsSheet by remember { mutableStateOf(false) }
    var showAddDeviceDialog by remember { mutableStateOf(false) }

    val deviceScreenState by deviceViewModel.state.collectAsState()
    val deviceUiState = deviceScreenState.deviceUiState
    val unreadNotificationsCount = deviceScreenState.notifications.count { !it.isRead }

    Box(modifier = Modifier.fillMaxSize()) {
        // 1. Hiển thị nội dung màn hình tương ứng
        when (activeScreen) {
            "OVERVIEW" -> {
                OverviewScreen(
                    deviceState = deviceUiState,
                    deviceCount = deviceScreenState.devices.size,
                    unreadCount = unreadNotificationsCount,
                    onTogglePump = { targetState ->
                        deviceViewModel.togglePump(targetState)
                    },
                    onDeviceClick = { deviceId ->
                        activeScreen = "DETAIL"
                    },
                    onNotificationClick = {
                        showNotificationsSheet = true
                    },
                    onAddDeviceClick = {
                        showAddDeviceDialog = true
                    }
                )
            }
            "ROOM_GRID" -> {
                RoomGridScreen(
                    roomName = deviceUiState.name.ifBlank { "Tủ Bơm & Bể Nước" },
                    deviceState = deviceUiState,
                    onTogglePump = { targetState ->
                        deviceViewModel.togglePump(targetState)
                    },
                    onToggleChildLock = { isLock ->
                        deviceViewModel.toggleChildLock(isLock)
                    },
                    onBackClick = {
                        activeScreen = "OVERVIEW"
                        currentTab = NavigationTab.HOME
                    },
                    onGoToControlClick = {
                        activeScreen = "DETAIL"
                        currentTab = NavigationTab.CONTROL
                    },
                    onDeviceClick = { deviceId ->
                        activeScreen = "DETAIL"
                    }
                )
            }
            "DETAIL" -> {
                DetailControlScreen(
                    deviceState = deviceUiState,
                    logs = deviceScreenState.logs,
                    onTogglePump = { targetState ->
                        deviceViewModel.togglePump(targetState)
                    },
                    onToggleChildLock = { isLock ->
                        deviceViewModel.toggleChildLock(isLock)
                    },
                    onToggleAutoMode = { isAuto ->
                        deviceViewModel.toggleAutoMode(isAuto)
                    },
                    onSaveTankConfig = { height, offset, minP, maxP ->
                        deviceViewModel.saveTankConfig(height, offset, minP, maxP)
                    },
                    onRefreshLogs = {
                        deviceViewModel.loadLogs()
                    },
                    onShareDevice = { email, perm ->
                        deviceViewModel.shareDevice(email, perm)
                    },
                    onDeleteDevice = { deviceId ->
                        deviceViewModel.deleteDevice(deviceId)
                    },
                    onBackClick = {
                        activeScreen = "OVERVIEW"
                        currentTab = NavigationTab.HOME
                    }
                )
            }
            "ADMIN" -> {
                AdminDashboardScreen(
                    deviceState = deviceUiState,
                    logs = deviceScreenState.logs,
                    onSaveTankConfig = { height, offset, minP, maxP ->
                        deviceViewModel.saveTankConfig(height, offset, minP, maxP)
                    },
                    onBackClick = {
                        activeScreen = "OVERVIEW"
                        currentTab = NavigationTab.HOME
                    },
                    onLogout = onLogout
                )
            }
        }

        // 2. Thanh điều hướng nổi (Floating Bottom Bar)
        FloatingBottomBar(
            currentTab = currentTab,
            onTabSelected = { selectedTab ->
                currentTab = selectedTab
                activeScreen = when (selectedTab) {
                    NavigationTab.HOME -> "OVERVIEW"
                    NavigationTab.CONTROL -> "DETAIL"
                    NavigationTab.DEVICES -> "ROOM_GRID"
                    NavigationTab.ADMIN -> "ADMIN"
                }
            },
            modifier = Modifier.align(Alignment.BottomCenter)
        )

        // 3. Banner thông báo trượt từ đỉnh màn hình xuống (3 giây tự thu lại)
        com.example.androi.ui.components.common.TopNotificationBanner(
            notification = deviceScreenState.activeBanner,
            onDismiss = { deviceViewModel.dismissBanner() },
            modifier = Modifier.align(Alignment.TopCenter)
        )
    }

    // 4. Hộp thoại BottomSheet hiển thị danh sách thông báo khi bấm icon Cái Chuông
    if (showNotificationsSheet) {
        com.example.androi.ui.components.common.NotificationsBottomSheet(
            notifications = deviceScreenState.notifications,
            onDismiss = { showNotificationsSheet = false },
            onClearAll = { deviceViewModel.clearNotifications() }
        )
    }

    // 5. Hộp thoại Thêm Máy Bơm Mới khi bấm nút (+)
    if (showAddDeviceDialog) {
        com.example.androi.ui.components.home.AddDeviceDialog(
            onDismiss = { showAddDeviceDialog = false },
            onConfirm = { code, name ->
                showAddDeviceDialog = false
                deviceViewModel.createDevice(code, name)
            }
        )
    }
}
