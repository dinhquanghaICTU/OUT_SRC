package com.example.androi.ui.components.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.PowerSettingsNew
import androidx.compose.material.icons.rounded.Schedule
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.ui.components.common.SmartSwitch
import com.example.androi.ui.theme.*
import kotlinx.coroutines.delay

import androidx.compose.foundation.border
import androidx.compose.material.icons.rounded.Lock

/**
 * Card Điều Khiển Bơm: Nút Bật/Tắt + Switch trạng thái + Thông số thời gian bật bơm
 */
@Composable
fun PumpControlCard(
    isPumpRunning: Boolean,
    pumpRuntimeSec: Int = 0,
    isAutoMode: Boolean = false,
    isOnline: Boolean = true,
    onTogglePump: (Boolean) -> Unit,
    modifier: Modifier = Modifier
) {
    // Đếm thời gian thực khi máy bơm đang chạy
    var liveSeconds by remember(isPumpRunning, pumpRuntimeSec) {
        mutableIntStateOf(if (isPumpRunning && pumpRuntimeSec == 0) 1 else pumpRuntimeSec)
    }

    LaunchedEffect(isPumpRunning) {
        if (isPumpRunning) {
            while (true) {
                delay(1000)
                liveSeconds++
            }
        }
    }

    Box(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp)
            .clip(RoundedCornerShape(28.dp))
            .background(CardBackground)
            .padding(20.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Phần thông tin bên trái có weight(1f) để không đẩy văng switch
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier
                    .weight(1f)
                    .padding(end = 12.dp)
            ) {
                Box(
                    modifier = Modifier
                        .size(50.dp)
                        .clip(CircleShape)
                        .background(if (isPumpRunning && isOnline) BrandOrange else SurfaceVariant)
                        .clickable(enabled = isOnline && !isAutoMode) { onTogglePump(!isPumpRunning) },
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = Icons.Rounded.PowerSettingsNew,
                        contentDescription = "Power",
                        tint = if (isPumpRunning) Color.White else TextSecondary,
                        modifier = Modifier.size(26.dp)
                    )
                }

                Spacer(modifier = Modifier.width(14.dp))

                Column {
                    Text(
                        text = "Động Cơ Máy Bơm",
                        fontSize = 16.sp,
                        fontWeight = FontWeight.Bold,
                        color = TextPrimary,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    Spacer(modifier = Modifier.height(2.dp))
                    Text(
                        text = if (isPumpRunning) "Đang hoạt động (Bơm nước)" else "Đang dừng • Sẵn sàng",
                        fontSize = 13.sp,
                        color = if (isPumpRunning) BrandOrange else TextSecondary,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )

                    // Hiển thị thông số thời gian bật bơm
                    Spacer(modifier = Modifier.height(4.dp))
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(4.dp)
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.Schedule,
                            contentDescription = null,
                            tint = if (isPumpRunning) BrandOrange else TextSecondary,
                            modifier = Modifier.size(13.dp)
                        )
                        Text(
                            text = if (isPumpRunning) {
                                "Thời gian chạy: ${formatDuration(liveSeconds)}"
                            } else if (pumpRuntimeSec > 0) {
                                "Lần bơm gần nhất: ${formatDuration(pumpRuntimeSec)}"
                            } else {
                                "Thời gian bơm: 0s"
                            },
                            fontSize = 11.sp,
                            fontWeight = if (isPumpRunning) FontWeight.SemiBold else FontWeight.Normal,
                            color = if (isPumpRunning) BrandOrange else TextSecondary
                        )
                    }
                }
            }

            // Switch nhanh ON / OFF hoặc Badge AUTO khi ở chế độ tự động
            Box(contentAlignment = Alignment.CenterEnd) {
                if (isAutoMode) {
                    Box(
                        modifier = Modifier
                            .clip(RoundedCornerShape(20.dp))
                            .background(SurfaceVariant)
                            .border(1.dp, Color(0xFF3B82F6).copy(alpha = 0.3f), RoundedCornerShape(20.dp))
                            .clickable { onTogglePump(isPumpRunning) }
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
                                text = if (isPumpRunning) "AUTO ON" else "AUTO",
                                fontSize = 11.sp,
                                fontWeight = FontWeight.Bold,
                                color = if (isPumpRunning) BrandOrange else Color(0xFF3B82F6)
                            )
                        }
                    }
                } else {
                    SmartSwitch(
                        checked = if (isOnline) isPumpRunning else false,
                        enabled = isOnline,
                        onCheckedChange = onTogglePump
                    )
                }
            }
        }
    }
}

private fun formatDuration(seconds: Int): String {
    val hrs = seconds / 3600
    val mins = (seconds % 3600) / 60
    val secs = seconds % 60
    return when {
        hrs > 0 -> String.format("%02d:%02d:%02d", hrs, mins, secs)
        mins > 0 -> String.format("%02d phút %02d giây", mins, secs)
        else -> "${secs} giây"
    }
}

