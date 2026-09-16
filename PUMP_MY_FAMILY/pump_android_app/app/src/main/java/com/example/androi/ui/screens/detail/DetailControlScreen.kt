package com.example.androi.ui.screens.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.DeleteOutline
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.DeviceUiState
import com.example.androi.data.model.PumpLogDto
import com.example.androi.ui.components.common.ShareDeviceDialog
import com.example.androi.ui.components.detail.*
import com.example.androi.ui.theme.ScreenBackground

/**
 * Màn hình Chi Tiết Thiết Bị (Detail Screen):
 * Lắp ráp từ các Component độc lập trong package ui.components.detail:
 * 1. DetailTopBar: Nút Back + Tiêu đề + Trực tuyến ESP32 + Chia sẻ thiết bị + Cài đặt
 * 2. DetailModeSelector: Tự Động / Thủ Công (Đã bỏ Hẹn Giờ)
 * 3. WaterTankStatusCard: Đồng hồ cánh cung Arc Gauge, % nước, khoảng cách cm, dung tích lít, pin ESP-NOW
 * 4. PumpControlCard: Nút công tắc điều khiển động cơ bơm
 * 5. ChildLockCard: Khóa trẻ em bảo vệ nút bấm vật lý tủ điện
 * 6. AutoThresholdCard: Cài đặt ngưỡng tự động Bật/Tắt
 * 7. PumpLogsCard: Nhật ký vận hành bơm đồng bộ trực tiếp với Web
 */
@Composable
fun DetailControlScreen(
    deviceState: DeviceUiState = DeviceUiState(),
    logs: List<PumpLogDto> = emptyList(),
    onTogglePump: (Boolean) -> Unit = {},
    onToggleChildLock: (Boolean) -> Unit = {},
    onToggleAutoMode: (Boolean) -> Unit = {},
    onSaveTankConfig: (Float, Float, Int, Int) -> Unit = { _, _, _, _ -> },
    onRefreshLogs: () -> Unit = {},
    onShareDevice: (String, String) -> Unit = { _, _ -> },
    onDeleteDevice: (String) -> Unit = {},
    onBackClick: () -> Unit = {},
    modifier: Modifier = Modifier
) {
    val selectedMode = if (deviceState.isAutoMode) 0 else 1
    var showShareDialog by remember { mutableStateOf(false) }
    var showDeleteDialog by remember { mutableStateOf(false) }

    if (showShareDialog) {
        ShareDeviceDialog(
            deviceName = deviceState.name.ifBlank { "Máy Bơm Gia Đình" },
            onDismiss = { showShareDialog = false },
            onShare = { email, permission ->
                onShareDevice(email, permission)
            }
        )
    }

    if (showDeleteDialog) {
        com.example.androi.ui.components.detail.DeleteDeviceDialog(
            deviceName = deviceState.name.ifBlank { "Máy Bơm" },
            onDismiss = { showDeleteDialog = false },
            onConfirmDelete = {
                showDeleteDialog = false
                onDeleteDevice(deviceState.id)
                onBackClick()
            }
        )
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .background(ScreenBackground)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 170.dp)
    ) {
        // 1. Top Bar với nút Chia sẻ thiết bị & Nút Xóa thiết bị
        DetailTopBar(
            title = deviceState.name.ifBlank { "Chi Tiết Máy Bơm" },
            isOnline = deviceState.isOnline,
            onBackClick = onBackClick,
            onShareClick = { showShareDialog = true },
            onDeleteClick = { showDeleteDialog = true }
        )

        Spacer(modifier = Modifier.height(8.dp))

        // 2. Chế độ vận hành (Tự Động / Thủ Công)
        DetailModeSelector(
            selectedModeIndex = selectedMode,
            onModeSelected = { index ->
                onToggleAutoMode(index == 0)
            }
        )

        Spacer(modifier = Modifier.height(18.dp))

        // 3. Card trạng thái bể nước (Arc Gauge, %, khoảng cách cm, lít, pin)
        WaterTankStatusCard(
            waterPercent = deviceState.waterPercent,
            distanceCm = deviceState.distanceCm,
            batteryVoltage = deviceState.batteryVoltage
        )

        Spacer(modifier = Modifier.height(18.dp))

        // 4. Card công tắc điều khiển máy bơm & thông số thời gian chạy
        PumpControlCard(
            isPumpRunning = deviceState.isPumpRunning,
            pumpRuntimeSec = deviceState.pumpRuntimeSec,
            isAutoMode = deviceState.isAutoMode,
            isOnline = deviceState.isOnline,
            onTogglePump = onTogglePump
        )

        Spacer(modifier = Modifier.height(18.dp))

        // 5. Card tính năng Khóa Trẻ Em (Child Lock)
        ChildLockCard(
            isChildLock = deviceState.isChildLock,
            onToggleChildLock = onToggleChildLock
        )

        Spacer(modifier = Modifier.height(18.dp))

        // 6. Card cấu hình ngưỡng tự động
        AutoThresholdCard(
            onThresholdPercent = deviceState.minWaterPercent,
            offThresholdPercent = deviceState.maxWaterPercent
        )

        Spacer(modifier = Modifier.height(18.dp))

        // 7. Nhật ký lịch sử vận hành đồng bộ trực tiếp với Web
        PumpLogsCard(
            logs = logs,
            isOnline = deviceState.isOnline,
            onRefreshLogs = onRefreshLogs
        )

        Spacer(modifier = Modifier.height(20.dp))

        // 8. Nút Xóa Máy Bơm Khỏi Tài Khoản
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp)
                .clip(androidx.compose.foundation.shape.RoundedCornerShape(22.dp))
                .background(Color(0xFFFEF2F2))
                .clickable { showDeleteDialog = true }
                .padding(vertical = 15.dp),
            contentAlignment = Alignment.Center
        ) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Icon(
                    imageVector = androidx.compose.material.icons.Icons.Rounded.DeleteOutline,
                    contentDescription = null,
                    tint = Color(0xFFEF4444),
                    modifier = Modifier.size(20.dp)
                )
                Text(
                    text = "Xóa Máy Bơm Này Khỏi Danh Sách",
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Bold,
                    color = Color(0xFFDC2626)
                )
            }
        }
    }
}
