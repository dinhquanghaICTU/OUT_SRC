package com.example.androi.ui.components.common

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.AppNotification
import com.example.androi.data.model.NotificationType
import com.example.androi.ui.theme.BrandOrange
import com.example.androi.ui.theme.CardBackground
import com.example.androi.ui.theme.TextPrimary
import com.example.androi.ui.theme.TextSecondary

/**
 * Bottom Sheet danh sách tất cả thông báo hệ thống khi người dùng bấm vào icon cái Chuông
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun NotificationsBottomSheet(
    notifications: List<AppNotification>,
    onDismiss: () -> Unit,
    onClearAll: () -> Unit,
    onNotificationClick: (AppNotification) -> Unit = {}
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = CardBackground,
        shape = RoundedCornerShape(topStart = 32.dp, topEnd = 32.dp),
        dragHandle = {
            Box(
                modifier = Modifier
                    .padding(vertical = 12.dp)
                    .width(40.dp)
                    .height(4.dp)
                    .clip(CircleShape)
                    .background(Color(0xFFCBD5E1))
            )
        }
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp)
                .padding(bottom = 36.dp)
        ) {
            // Header BottomSheet
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
                            imageVector = Icons.Rounded.Notifications,
                            contentDescription = null,
                            tint = BrandOrange,
                            modifier = Modifier.size(20.dp)
                        )
                    }

                    Column {
                        Text(
                            text = "Thông Báo Hệ Thống",
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Text(
                            text = "${notifications.size} thông báo đã ghi nhận",
                            fontSize = 12.sp,
                            color = TextSecondary
                        )
                    }
                }

                if (notifications.isNotEmpty()) {
                    TextButton(onClick = onClearAll) {
                        Text(
                            text = "Xóa tất cả",
                            fontSize = 13.sp,
                            fontWeight = FontWeight.SemiBold,
                            color = BrandOrange
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(18.dp))

            // Danh sách thông báo
            if (notifications.isEmpty()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 40.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(
                            imageVector = Icons.Rounded.NotificationsNone,
                            contentDescription = null,
                            tint = Color(0xFFCBD5E1),
                            modifier = Modifier.size(56.dp)
                        )
                        Spacer(modifier = Modifier.height(12.dp))
                        Text(
                            text = "Chưa có thông báo nào",
                            fontSize = 15.sp,
                            fontWeight = FontWeight.SemiBold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "Mọi thông báo mực nước đầy, cạn và bật tắt bơm sẽ hiển thị ở đây.",
                            fontSize = 12.sp,
                            color = TextSecondary,
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    }
                }
            } else {
                LazyColumn(
                    verticalArrangement = Arrangement.spacedBy(10.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    items(notifications, key = { it.id }) { item ->
                        NotificationListItem(
                            notification = item,
                            onClick = { onNotificationClick(item) }
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun NotificationListItem(
    notification: AppNotification,
    onClick: () -> Unit
) {
    val (icon, iconBg, iconColor) = when (notification.type) {
        NotificationType.WATER_FULL -> Triple(
            Icons.Rounded.WaterDrop,
            Color(0xFF0284C7).copy(alpha = 0.12f),
            Color(0xFF0284C7)
        )
        NotificationType.WATER_LOW -> Triple(
            Icons.Rounded.WarningAmber,
            BrandOrange.copy(alpha = 0.12f),
            BrandOrange
        )
        NotificationType.PUMP_ON -> Triple(
            Icons.Rounded.PowerSettingsNew,
            Color(0xFF10B981).copy(alpha = 0.12f),
            Color(0xFF059669)
        )
        NotificationType.PUMP_OFF -> Triple(
            Icons.Rounded.PowerOff,
            Color(0xFFEF4444).copy(alpha = 0.12f),
            Color(0xFFDC2626)
        )
        NotificationType.WARNING -> Triple(
            Icons.Rounded.ReportProblem,
            Color(0xFFF59E0B).copy(alpha = 0.12f),
            Color(0xFFD97706)
        )
        NotificationType.INFO -> Triple(
            Icons.Rounded.NotificationsActive,
            Color(0xFF6366F1).copy(alpha = 0.12f),
            Color(0xFF4F46E5)
        )
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(18.dp))
            .background(Color(0xFFF8FAFC))
            .clickable { onClick() }
            .padding(14.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Box(
            modifier = Modifier
                .size(42.dp)
                .clip(CircleShape)
                .background(iconBg),
            contentAlignment = Alignment.Center
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                tint = iconColor,
                modifier = Modifier.size(20.dp)
            )
        }

        Column(modifier = Modifier.weight(1f)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = notification.title,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )

                Text(
                    text = formatRelativeTime(notification.timestamp),
                    fontSize = 11.sp,
                    color = TextSecondary
                )
            }

            Spacer(modifier = Modifier.height(3.dp))

            Text(
                text = notification.message,
                fontSize = 12.sp,
                color = TextSecondary,
                lineHeight = 17.sp
            )
        }
    }
}

private fun formatRelativeTime(timeMs: Long): String {
    val diffSec = (System.currentTimeMillis() - timeMs) / 1000
    return when {
        diffSec < 10 -> "Vừa xong"
        diffSec < 60 -> "${diffSec}s trước"
        diffSec < 3600 -> "${diffSec / 60} phút trước"
        diffSec < 86400 -> "${diffSec / 3600} giờ trước"
        else -> "${diffSec / 86400} ngày trước"
    }
}
