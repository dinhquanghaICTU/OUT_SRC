package com.example.androi.ui.components.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Info
import androidx.compose.material.icons.rounded.Lock
import androidx.compose.material.icons.rounded.LockOpen
import androidx.compose.material.icons.rounded.Shield
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.ui.components.common.SmartSwitch
import com.example.androi.ui.theme.*

/**
 * Card Tính Năng Khóa Trẻ Em (Child Lock):
 * - Bật / Tắt Khóa nút bấm vật lý trên tủ điện ESP32
 * - Hiển thị banner cảnh báo trạng thái an toàn
 */
@Composable
fun ChildLockCard(
    isChildLock: Boolean,
    onToggleChildLock: (Boolean) -> Unit,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp)
            .clip(RoundedCornerShape(28.dp))
            .background(CardBackground)
            .padding(20.dp)
    ) {
        Column {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.weight(1f)
                ) {
                    Box(
                        modifier = Modifier
                            .size(46.dp)
                            .clip(CircleShape)
                            .background(if (isChildLock) Color(0xFFEF4444) else SurfaceVariant),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = if (isChildLock) Icons.Rounded.Lock else Icons.Rounded.LockOpen,
                            contentDescription = "Child Lock",
                            tint = if (isChildLock) Color.White else TextSecondary,
                            modifier = Modifier.size(22.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(14.dp))

                    Column {
                        Text(
                            text = "Khóa Trẻ Em (Child Lock)",
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(2.dp))
                        Text(
                            text = "Khóa nút bấm cơ tại tủ điện ngoài sân",
                            fontSize = 12.sp,
                            color = TextSecondary
                        )
                    }
                }

                SmartSwitch(
                    checked = isChildLock,
                    onCheckedChange = onToggleChildLock
                )
            }

            Spacer(modifier = Modifier.height(14.dp))

            // Cảnh báo trạng thái Khóa Trẻ Em
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(if (isChildLock) Color(0xFFFEF2F2) else SurfaceVariant)
                    .padding(horizontal = 14.dp, vertical = 10.dp)
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        imageVector = if (isChildLock) Icons.Rounded.Shield else Icons.Rounded.Info,
                        contentDescription = null,
                        tint = if (isChildLock) Color(0xFFEF4444) else TextSecondary,
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = if (isChildLock)
                            "ĐANG KHÓA AN TOÀN: Nút cứng tại tủ ESP32 bị vô hiệu hóa."
                        else
                            "ĐANG MỞ: Có thể ấn nút trực tiếp trên tủ để bật máy bơm.",
                        fontSize = 12.sp,
                        color = if (isChildLock) Color(0xFFDC2626) else TextSecondary,
                        fontWeight = if (isChildLock) FontWeight.Medium else FontWeight.Normal
                    )
                }
            }
        }
    }
}
