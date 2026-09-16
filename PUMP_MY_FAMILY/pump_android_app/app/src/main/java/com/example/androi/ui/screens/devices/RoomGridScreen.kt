package com.example.androi.ui.screens.devices

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ArrowBack
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.DeviceCardItem
import com.example.androi.data.model.DeviceIconType
import com.example.androi.data.model.DeviceUiState
import com.example.androi.ui.components.home.DeviceGridCard
import com.example.androi.ui.theme.*

/**
 * Màn hình 2: Lưới thiết bị 2x2 theo khu vực (Screen 2 - Living Room / Devices Grid)
 * Đã dựng sẵn UI 100% khớp template để bạn tự xử lý sự kiện
 */
@Composable
fun RoomGridScreen(
    roomName: String = "Tủ Bơm & Bể Nước",
    deviceState: DeviceUiState = DeviceUiState(),
    onTogglePump: (Boolean) -> Unit = {},
    onToggleChildLock: (Boolean) -> Unit = {},
    onBackClick: () -> Unit = {},
    onGoToControlClick: () -> Unit = onBackClick,
    onDeviceClick: (String) -> Unit = {},
    modifier: Modifier = Modifier
) {
    val devices = remember(deviceState) {
        listOf(
            DeviceCardItem(
                id = "pump",
                title = "Máy Bơm Chính",
                subtitle = "Tủ Điện S3",
                iconType = DeviceIconType.PUMP,
                isRunning = deviceState.isPumpRunning,
                statusText = if (deviceState.isPumpRunning) "Đang Bật" else "Đang Tắt"
            ),
            DeviceCardItem(
                id = "tank",
                title = "Cảm Biến Bể",
                subtitle = if (deviceState.isTankOnline) "Sóng ESP-NOW" else "⚠️ Mất Kết Nối",
                iconType = DeviceIconType.WATER_TANK,
                isRunning = deviceState.isTankOnline,
                statusText = "Nước: ${"%.1f".format(deviceState.waterPercent)}%"
            ),
            DeviceCardItem(
                id = "lock",
                title = "Khóa An Toàn",
                subtitle = "Khóa nút vật lý",
                iconType = DeviceIconType.CHILD_LOCK,
                isRunning = deviceState.isChildLock,
                statusText = if (deviceState.isChildLock) "ĐANG KHÓA" else "MỞ KHÓA"
            ),
            DeviceCardItem(
                id = "solar",
                title = "Nguồn Pin Node",
                subtitle = "Bể Nước",
                iconType = DeviceIconType.SOLAR_BATTERY,
                isRunning = deviceState.batteryVoltage > 3.5f,
                statusText = "Pin: ${"%.2f".format(deviceState.batteryVoltage)}V"
            )
        )
    }

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(ScreenBackground)
    ) {
        // 1. Nội dung bên dưới được làm mờ (Blur / Dimmed Effect)
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(bottom = 100.dp)
                .alpha(0.35f)
        ) {
            // Top Bar
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp, vertical = 16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(44.dp)
                        .clip(CircleShape)
                        .background(CardBackground)
                        .clickable { onBackClick() },
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Rounded.ArrowBack,
                        contentDescription = "Back",
                        tint = TextPrimary,
                        modifier = Modifier.size(20.dp)
                    )
                }

                Text(
                    text = roomName,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )

                Spacer(modifier = Modifier.size(44.dp))
            }

            // Banner Card
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp)
                    .height(170.dp)
                    .clip(RoundedCornerShape(32.dp))
                    .background(Color(0xFFE3E8EC)),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Icon(
                        imageVector = Icons.Rounded.HomeWork,
                        contentDescription = "Room Banner",
                        tint = BrandOrange.copy(alpha = 0.7f),
                        modifier = Modifier.size(56.dp)
                    )
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(
                        text = "Hệ thống Bơm & Bể Nước Gia Đình",
                        fontSize = 14.sp,
                        fontWeight = FontWeight.Medium,
                        color = TextPrimary
                    )
                }

                Row(
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .padding(bottom = 16.dp)
                        .clip(RoundedCornerShape(24.dp))
                        .background(Color.White.copy(alpha = 0.85f))
                        .padding(horizontal = 14.dp, vertical = 6.dp),
                    horizontalArrangement = Arrangement.spacedBy(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(Icons.Rounded.Videocam, contentDescription = null, tint = TextPrimary, modifier = Modifier.size(18.dp))
                    Icon(Icons.Rounded.CropFree, contentDescription = null, tint = TextPrimary, modifier = Modifier.size(18.dp))
                    Icon(Icons.Rounded.RadioButtonChecked, contentDescription = null, tint = BrandOrange, modifier = Modifier.size(18.dp))
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Tiêu đề "Thiết bị của tôi"
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Thiết bị của tôi",
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )
                Text(
                    text = "Xem tất cả",
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Medium,
                    color = TextSecondary
                )
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Lưới 2x2 thiết bị
            LazyVerticalGrid(
                columns = GridCells.Fixed(2),
                contentPadding = PaddingValues(horizontal = 24.dp),
                horizontalArrangement = Arrangement.spacedBy(16.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp),
                modifier = Modifier.fillMaxWidth()
            ) {
                items(devices.size) { index ->
                    val item = devices[index]
                    DeviceGridCard(
                        item = item,
                        onToggle = { isChecked ->
                            when (item.id) {
                                "pump" -> onTogglePump(isChecked)
                                "lock" -> onToggleChildLock(isChecked)
                            }
                        },
                        onCardClick = { onDeviceClick(item.id) }
                    )
                }
            }
        }

        // 2. Lớp phủ Card Thông Báo Bảo Trì (Maintenance Overlay Card)
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(bottom = 100.dp, start = 24.dp, end = 24.dp),
            contentAlignment = Alignment.Center
        ) {
            Card(
                shape = RoundedCornerShape(32.dp),
                colors = CardDefaults.cardColors(containerColor = CardBackground),
                elevation = CardDefaults.cardElevation(defaultElevation = 16.dp),
                modifier = Modifier.fillMaxWidth()
            ) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(28.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    // Icon cờ lê / công cụ bảo trì
                    Box(
                        modifier = Modifier
                            .size(72.dp)
                            .clip(CircleShape)
                            .background(BrandOrange.copy(alpha = 0.12f)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.Construction,
                            contentDescription = "Maintenance",
                            tint = BrandOrange,
                            modifier = Modifier.size(36.dp)
                        )
                    }

                    Spacer(modifier = Modifier.height(18.dp))

                    // Badge trạng thái
                    Box(
                        modifier = Modifier
                            .clip(RoundedCornerShape(8.dp))
                            .background(BrandOrange.copy(alpha = 0.15f))
                            .padding(horizontal = 12.dp, vertical = 5.dp)
                    ) {
                        Text(
                            text = "HỆ THỐNG ĐANG BẢO TRÌ",
                            fontSize = 11.sp,
                            fontWeight = FontWeight.Bold,
                            color = BrandOrange,
                            letterSpacing = 0.5.sp
                        )
                    }

                    Spacer(modifier = Modifier.height(10.dp))

                    Text(
                        text = "Tính Năng Đang Nâng Cấp",
                        fontSize = 20.sp,
                        fontWeight = FontWeight.Bold,
                        color = TextPrimary
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    Text(
                        text = "Giao diện quản lý lưới đa thiết bị đang được hoàn thiện để đồng bộ chuẩn ESP-NOW đa trạm.\n\nQuý khách vui lòng sang tab Điều Khiển để xem mực nước và bật tắt máy bơm chính.",
                        fontSize = 13.sp,
                        color = TextSecondary,
                        lineHeight = 20.sp,
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )

                    Spacer(modifier = Modifier.height(22.dp))

                    Button(
                        onClick = onGoToControlClick,
                        shape = RoundedCornerShape(16.dp),
                        colors = ButtonDefaults.buttonColors(containerColor = BrandOrange),
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(48.dp)
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.Tune,
                            contentDescription = null,
                            tint = Color.White,
                            modifier = Modifier.size(18.dp)
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(
                            text = "Về Màn Hình Điều Khiển",
                            fontSize = 14.sp,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                    }
                }
            }
        }
    }
}
