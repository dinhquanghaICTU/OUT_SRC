package com.example.androi.ui.components.detail

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.History
import androidx.compose.material.icons.rounded.Refresh
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.PumpLogDto
import com.example.androi.ui.theme.*

/**
 * Card hiển thị lịch sử bật tắt máy bơm đồng bộ thời gian thực với Web Dashboard
 * API: GET /api/pumps/{device_id}/logs
 */
@Composable
fun PumpLogsCard(
    logs: List<PumpLogDto> = emptyList(),
    isOnline: Boolean = true,
    onRefreshLogs: () -> Unit = {},
    modifier: Modifier = Modifier
) {
    var isRefreshing by remember { mutableStateOf(false) }
    val rotation by animateFloatAsState(
        targetValue = if (isRefreshing) 360f else 0f,
        animationSpec = tween(durationMillis = 600, easing = LinearEasing),
        label = "refreshRotate",
        finishedListener = { isRefreshing = false }
    )

    Box(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp)
            .clip(RoundedCornerShape(26.dp))
            .background(CardBackground)
            .padding(20.dp)
    ) {
        Column {
            // Header Card: Tiêu đề + Nút Đồng bộ Web
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(10.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .size(38.dp)
                            .clip(CircleShape)
                            .background(BrandOrange.copy(alpha = 0.15f)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.History,
                            contentDescription = null,
                            tint = BrandOrange,
                            modifier = Modifier.size(20.dp)
                        )
                    }

                    Column {
                        Text(
                            text = "Lịch Sử Vận Hành Bơm",
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Text(
                            text = "Đồng bộ dữ liệu với Web Dashboard",
                            fontSize = 11.sp,
                            color = TextSecondary
                        )
                    }
                }

                // Nút Sync / Refresh (Bị disable khi thiết bị đang Offline hoặc đang Refresh)
                IconButton(
                    onClick = {
                        isRefreshing = true
                        onRefreshLogs()
                    },
                    enabled = isOnline && !isRefreshing,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        imageVector = Icons.Rounded.Refresh,
                        contentDescription = "Sync Logs",
                        tint = if (isOnline) BrandOrange else TextMuted.copy(alpha = 0.4f),
                        modifier = Modifier
                            .size(22.dp)
                            .rotate(rotation)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Danh sách nhật ký
            if (logs.isEmpty()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 20.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "Chưa có nhật ký vận hành nào từ máy chủ.",
                        fontSize = 13.sp,
                        color = TextSecondary
                    )
                }
            } else {
                Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    logs.take(6).forEach { log ->
                        PumpLogItemRow(log = log)
                    }
                }
            }
        }
    }
}

@Composable
private fun PumpLogItemRow(log: PumpLogDto) {
    val isPumpOn = log.action.contains("ON", ignoreCase = true)
    val badgeBg = if (isPumpOn) Color(0xFF10B981).copy(alpha = 0.15f) else Color(0xFFEF4444).copy(alpha = 0.15f)
    val badgeColor = if (isPumpOn) Color(0xFF059669) else Color(0xFFDC2626)
    val actionText = if (isPumpOn) "BẬT BƠM" else if (log.action.contains("OFF", ignoreCase = true)) "TẮT BƠM" else log.action

    val userDisplay = when (val tb = log.triggeredBy) {
        is String -> tb
        is Map<*, *> -> "${tb["full_name"] ?: ""} (${tb["email"] ?: ""})".trim()
        null -> "Tự Động (Auto)"
        else -> tb.toString()
    }

    // Rút gọn thời gian dạng 2026-09-08T18:45:00 -> 18:45 - 08/09
    val formattedTime = formatLogTime(log.createdAt)

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(14.dp))
            .background(Color(0xFFF8FAFC))
            .padding(horizontal = 12.dp, vertical = 10.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                // Badge BẬT BƠM / TẮT BƠM
                Box(
                    modifier = Modifier
                        .clip(RoundedCornerShape(6.dp))
                        .background(badgeBg)
                        .padding(horizontal = 8.dp, vertical = 2.dp)
                ) {
                    Text(
                        text = actionText,
                        fontSize = 10.sp,
                        fontWeight = FontWeight.Bold,
                        color = badgeColor
                    )
                }

                Text(
                    text = formattedTime,
                    fontSize = 11.sp,
                    color = TextSecondary
                )
            }

            Spacer(modifier = Modifier.height(3.dp))

            Text(
                text = userDisplay,
                fontSize = 12.sp,
                fontWeight = FontWeight.Medium,
                color = TextPrimary
            )
        }

        // Mực nước lúc thực hiện
        Column(horizontalAlignment = Alignment.End) {
            Text(
                text = if (log.waterLevel != null) "${"%.1f".format(log.waterLevel)}%" else "--",
                fontSize = 13.sp,
                fontWeight = FontWeight.Bold,
                color = BrandOrange
            )
            Text(
                text = "Mực nước",
                fontSize = 10.sp,
                color = TextSecondary
            )
        }
    }
}

private fun formatLogTime(raw: String): String {
    return try {
        // raw dạng ISO: 2026-09-08T18:45:12.123456
        if (raw.contains("T")) {
            val parts = raw.split("T")
            val dateParts = parts[0].split("-")
            val timeParts = parts[1].split(":")
            val time = "${timeParts[0]}:${timeParts[1]}"
            val date = "${dateParts[2]}/${dateParts[1]}"
            "$time $date"
        } else {
            raw.takeLast(14)
        }
    } catch (e: Exception) {
        raw
    }
}
