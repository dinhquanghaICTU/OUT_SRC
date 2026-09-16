package com.example.androi.ui.components.common

import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.AppNotification
import com.example.androi.data.model.NotificationType
import com.example.androi.ui.theme.BrandOrange
import com.example.androi.ui.theme.TextPrimary
import com.example.androi.ui.theme.TextSecondary

/**
 * Banner thông báo nổi từ đỉnh màn hình trượt xuống (Dynamic Island / Floating Top Alert)
 * Tự động ẩn sau 3 giây hoặc có thể bấm để đóng ngay.
 */
@Composable
fun TopNotificationBanner(
    notification: AppNotification?,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    AnimatedVisibility(
        visible = notification != null,
        enter = slideInVertically(initialOffsetY = { -it * 2 }) + fadeIn(),
        exit = slideOutVertically(targetOffsetY = { -it * 2 }) + fadeOut(),
        modifier = modifier
    ) {
        if (notification != null) {
            val (icon, iconBg, iconColor) = when (notification.type) {
                NotificationType.WATER_FULL -> Triple(
                    Icons.Rounded.WaterDrop,
                    Color(0xFF0284C7).copy(alpha = 0.15f),
                    Color(0xFF0284C7)
                )
                NotificationType.WATER_LOW -> Triple(
                    Icons.Rounded.WarningAmber,
                    BrandOrange.copy(alpha = 0.15f),
                    BrandOrange
                )
                NotificationType.PUMP_ON -> Triple(
                    Icons.Rounded.PowerSettingsNew,
                    Color(0xFF10B981).copy(alpha = 0.15f),
                    Color(0xFF059669)
                )
                NotificationType.PUMP_OFF -> Triple(
                    Icons.Rounded.PowerOff,
                    Color(0xFFEF4444).copy(alpha = 0.15f),
                    Color(0xFFDC2626)
                )
                NotificationType.WARNING -> Triple(
                    Icons.Rounded.ReportProblem,
                    Color(0xFFF59E0B).copy(alpha = 0.15f),
                    Color(0xFFD97706)
                )
                NotificationType.INFO -> Triple(
                    Icons.Rounded.NotificationsActive,
                    Color(0xFF6366F1).copy(alpha = 0.15f),
                    Color(0xFF4F46E5)
                )
            }

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 38.dp)
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .shadow(
                            elevation = 16.dp,
                            shape = RoundedCornerShape(24.dp),
                            spotColor = Color.Black.copy(alpha = 0.25f)
                        )
                        .clip(RoundedCornerShape(24.dp))
                        .background(Color.White)
                        .clickable { onDismiss() }
                        .padding(horizontal = 16.dp, vertical = 14.dp),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(14.dp)
                ) {
                    // Icon tròn màu sắc tương ứng
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
                            modifier = Modifier.size(22.dp)
                        )
                    }

                    // Nội dung thông báo
                    Column(modifier = Modifier.weight(1f)) {
                        Text(
                            text = notification.title,
                            fontSize = 14.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(2.dp))
                        Text(
                            text = notification.message,
                            fontSize = 12.sp,
                            color = TextSecondary,
                            lineHeight = 16.sp
                        )
                    }

                    // Nút tắt 'x' nhỏ góc phải
                    Box(
                        modifier = Modifier
                            .size(28.dp)
                            .clip(CircleShape)
                            .background(Color(0xFFF1F5F9))
                            .clickable { onDismiss() },
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.Close,
                            contentDescription = "Close",
                            tint = TextSecondary,
                            modifier = Modifier.size(16.dp)
                        )
                    }
                }
            }
        }
    }
}
