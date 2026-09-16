package com.example.androi.ui.components.home

import androidx.compose.animation.animateColorAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.KeyboardArrowRight
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.DeviceUiState
import com.example.androi.ui.components.common.SmartSwitch
import com.example.androi.ui.theme.*

/**
 * Card Thiết Bị Đơn Lẻ tại Trang Chủ:
 * - Trạng thái Online / Offline nhanh của Tủ S3 và Node Bể
 * - Badge chế độ Tự Động (Auto) / Thủ Công (Manual)
 * - Switch điều khiển nhanh: Chặn bấm khi đang ở chế độ Tự Động (có thông báo)
 * - Lưới 4 thông số Telemetry: Mực nước, Pin node bể, Khoảng cách, Thời gian bơm
 * - Thanh dẫn xem chi tiết bể nước, khóa trẻ em & cài đặt
 */
@Composable
fun SingleDeviceCard(
    deviceState: DeviceUiState,
    onTogglePump: (Boolean) -> Unit,
    onCardClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp)
            .clip(RoundedCornerShape(28.dp))
            .background(CardBackground)
            .border(1.dp, BorderLight, RoundedCornerShape(28.dp))
            .clickable { onCardClick() }
            .padding(18.dp)
    ) {
        Column {
            // Hàng 1: Icon tròn + Tên thiết bị + Badge Online/Offline + Switch hoặc Badge Auto
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.weight(1f)
                ) {
                    // Icon thiết bị tròn
                    Box(
                        modifier = Modifier
                            .size(50.dp)
                            .clip(CircleShape)
                            .background(if (deviceState.isPumpRunning) BrandOrange else SurfaceVariant),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = if (deviceState.isPumpRunning) Icons.Rounded.Waves else Icons.Rounded.WaterDrop,
                            contentDescription = null,
                            tint = if (deviceState.isPumpRunning) Color.White else BrandOrange,
                            modifier = Modifier.size(26.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(14.dp))

                    Column {
                        // Tên thiết bị
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Text(
                                text = deviceState.name,
                                fontSize = 16.sp,
                                fontWeight = FontWeight.Bold,
                                color = TextPrimary,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }

                        Spacer(modifier = Modifier.height(3.dp))

                        // Hàng Badges Online/Offline S3 và Node Bể + Chế độ
                        Row(
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(5.dp)
                        ) {
                            // Badge Online / Offline S3
                            StatusBadge(
                                isOnline = deviceState.isOnline,
                                textOnline = "S3 On",
                                textOffline = "S3 Off"
                            )

                            // Badge Online / Offline Node Bể
                            StatusBadge(
                                isOnline = deviceState.isTankOnline,
                                textOnline = "Bể On",
                                textOffline = "Bể Off"
                            )

                            if (deviceState.isAutoMode) {
                                Box(
                                    modifier = Modifier
                                        .clip(RoundedCornerShape(6.dp))
                                        .background(Color(0xFF3B82F6).copy(alpha = 0.15f))
                                        .padding(horizontal = 6.dp, vertical = 2.dp)
                                ) {
                                    Text(
                                        text = "AUTO",
                                        fontSize = 10.sp,
                                        fontWeight = FontWeight.Bold,
                                        color = Color(0xFF3B82F6)
                                    )
                                }
                            }
                        }
                    }
                }

                Spacer(modifier = Modifier.width(8.dp))

                // Nút điều khiển nhanh bên phải:
                // Nếu đang ở chế độ AUTO: Hiển thị badge khóa AUTO, bấm vào sẽ báo notice cho user
                if (deviceState.isAutoMode) {
                    Box(
                        modifier = Modifier
                            .clip(RoundedCornerShape(20.dp))
                            .background(SurfaceVariant)
                            .border(1.dp, Color(0xFF3B82F6).copy(alpha = 0.3f), RoundedCornerShape(20.dp))
                            .clickable {
                                // Bấm vào thông báo máy đang ở chế độ AUTO không được bật tắt tay
                                onTogglePump(deviceState.isPumpRunning)
                            }
                            .padding(horizontal = 10.dp, vertical = 6.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                imageVector = Icons.Rounded.Lock,
                                contentDescription = "Khóa Auto",
                                tint = Color(0xFF3B82F6),
                                modifier = Modifier.size(13.dp)
                            )
                            Spacer(modifier = Modifier.width(4.dp))
                            Text(
                                text = if (deviceState.isPumpRunning) "AUTO ON" else "AUTO",
                                fontSize = 11.sp,
                                fontWeight = FontWeight.Bold,
                                color = if (deviceState.isPumpRunning) BrandOrange else Color(0xFF3B82F6)
                            )
                        }
                    }
                } else {
                    // Chế độ Thủ Công:
                    // Khi thiết bị đang Offline: Disable nút nhấn không cho bấm
                    SmartSwitch(
                        checked = if (deviceState.isOnline) deviceState.isPumpRunning else false,
                        enabled = deviceState.isOnline,
                        onCheckedChange = onTogglePump
                    )
                }
            }

            Spacer(modifier = Modifier.height(14.dp))

            // Hàng 2: Lưới 4 chỉ số quan sát nhanh: Mực Nước, Pin Node Bể, Khoảng Cách, Thời Gian Bơm
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                // 1. Mực Nước
                TelemetryChip(
                    modifier = Modifier.weight(1f),
                    icon = Icons.Rounded.WaterDrop,
                    iconTint = Color(0xFF0284C7),
                    label = "Mực Nước",
                    value = "${deviceState.waterPercent.toInt()}%"
                )

                // 2. Pin Node Bể
                TelemetryChip(
                    modifier = Modifier.weight(1f),
                    icon = Icons.Rounded.BatteryChargingFull,
                    iconTint = if (deviceState.isTankOnline && deviceState.batteryVoltage >= 3.7f) Color(0xFF10B981) else Color(0xFFEF4444),
                    label = "Pin Bể",
                    value = if (deviceState.isTankOnline && deviceState.batteryVoltage > 0) "%.2fV".format(deviceState.batteryVoltage) else "--"
                )

                // 3. Khoảng Cách
                TelemetryChip(
                    modifier = Modifier.weight(1f),
                    icon = Icons.Rounded.Straighten,
                    iconTint = Color(0xFF6366F1),
                    label = "K.Cách",
                    value = if (deviceState.isTankOnline && deviceState.distanceCm >= 0) "%.0f cm".format(deviceState.distanceCm) else "--"
                )

                // 4. Thời Gian Bơm
                TelemetryChip(
                    modifier = Modifier.weight(1f),
                    icon = Icons.Rounded.Schedule,
                    iconTint = if (deviceState.isPumpRunning) BrandOrange else Color(0xFFF59E0B),
                    label = "Thời Gian",
                    value = formatDuration(deviceState.pumpRuntimeSec)
                )
            }

            Spacer(modifier = Modifier.height(14.dp))

            // Hàng 3: Thanh chỉ dẫn mở trang chi tiết xem mức nước, khóa trẻ em & cài đặt
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(SurfaceVariant)
                    .clickable { onCardClick() }
                    .padding(horizontal = 14.dp, vertical = 10.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(
                        modifier = Modifier
                            .size(8.dp)
                            .clip(CircleShape)
                            .background(if (deviceState.isOnline) Color(0xFF10B981) else Color(0xFFEF4444))
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "Chạm để xem chi tiết & cài đặt",
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Medium,
                        color = TextPrimary
                    )
                }

                Icon(
                    imageVector = Icons.AutoMirrored.Rounded.KeyboardArrowRight,
                    contentDescription = "Chi tiết",
                    tint = BrandOrange,
                    modifier = Modifier.size(20.dp)
                )
            }
        }
    }
}

@Composable
private fun StatusBadge(
    isOnline: Boolean,
    textOnline: String,
    textOffline: String
) {
    val bg = if (isOnline) Color(0xFF10B981).copy(alpha = 0.15f) else Color(0xFFEF4444).copy(alpha = 0.15f)
    val color = if (isOnline) Color(0xFF10B981) else Color(0xFFEF4444)

    Box(
        modifier = Modifier
            .clip(RoundedCornerShape(8.dp))
            .background(bg)
            .padding(horizontal = 6.dp, vertical = 2.dp),
        contentAlignment = Alignment.Center
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(
                modifier = Modifier
                    .size(6.dp)
                    .clip(CircleShape)
                    .background(color)
            )
            Spacer(modifier = Modifier.width(4.dp))
            Text(
                text = if (isOnline) textOnline else textOffline,
                fontSize = 10.sp,
                fontWeight = FontWeight.SemiBold,
                color = color,
                maxLines = 1,
                softWrap = false
            )
        }
    }
}

@Composable
private fun TelemetryChip(
    icon: ImageVector,
    iconTint: Color,
    label: String,
    value: String,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .clip(RoundedCornerShape(14.dp))
            .background(SurfaceVariant)
            .padding(vertical = 8.dp, horizontal = 2.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Center
            ) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = iconTint,
                    modifier = Modifier.size(12.dp)
                )
                Spacer(modifier = Modifier.width(2.dp))
                Text(
                    text = label,
                    fontSize = 9.sp,
                    fontWeight = FontWeight.Medium,
                    color = TextSecondary,
                    maxLines = 1,
                    softWrap = false
                )
            }
            Spacer(modifier = Modifier.height(2.dp))
            Text(
                text = value,
                fontSize = 11.5.sp,
                fontWeight = FontWeight.Bold,
                color = TextPrimary,
                maxLines = 1,
                softWrap = false
            )
        }
    }
}

private fun formatDuration(seconds: Int): String {
    val hrs = seconds / 3600
    val mins = (seconds % 3600) / 60
    val secs = seconds % 60
    return when {
        hrs > 0 -> "${hrs}h ${mins}m"
        mins > 0 -> "${mins}m ${secs}s"
        secs > 0 -> "${secs}s"
        else -> "0s"
    }
}

