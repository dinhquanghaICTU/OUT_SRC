package com.example.androi.ui.components.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.ui.components.common.CircularArcGauge
import com.example.androi.ui.theme.*

/**
 * Card Trạng Thái Bể Nước:
 * - Đồng hồ bán nguyệt Arc Gauge hiển thị % nước
 * - Điện áp pin cảm biến ESP-NOW
 * - Khoảng cách nước đo được (cm)
 * - Dung tích nước ước tính (Lít)
 */
@Composable
fun WaterTankStatusCard(
    waterPercent: Float,
    distanceCm: Float,
    batteryVoltage: Float,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp)
            .clip(RoundedCornerShape(32.dp))
            .background(CardBackground)
            .padding(20.dp)
    ) {
        Column {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column {
                    Text(
                        text = "Trạng Thái Bể Nước",
                        fontSize = 18.sp,
                        fontWeight = FontWeight.Bold,
                        color = TextPrimary
                    )
                    Text(
                        text = "Cảm biến ESP-NOW Tầng Thượng",
                        fontSize = 13.sp,
                        color = TextSecondary
                    )
                }

                Box(
                    modifier = Modifier
                        .clip(RoundedCornerShape(12.dp))
                        .background(SurfaceVariant)
                        .padding(horizontal = 10.dp, vertical = 6.dp)
                ) {
                    Text(
                        text = "Pin: ${batteryVoltage}V",
                        fontSize = 12.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = BrandOrange
                    )
                }
            }

            Spacer(modifier = Modifier.height(8.dp))

            // Đồng hồ bán nguyệt hiển thị % nước
            CircularArcGauge(
                value = waterPercent,
                unit = "%",
                label = "Mực Nước Trong Bể"
            )

            Spacer(modifier = Modifier.height(16.dp))

            // Thống kê nhanh: Khoảng cách mặt nước & Thể tích ước tính
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .clip(RoundedCornerShape(18.dp))
                        .background(SurfaceVariant)
                        .padding(14.dp)
                ) {
                    Column {
                        Text(text = "Khoảng cách nước", fontSize = 12.sp, color = TextSecondary)
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "$distanceCm cm",
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                    }
                }

                Box(
                    modifier = Modifier
                        .weight(1f)
                        .clip(RoundedCornerShape(18.dp))
                        .background(SurfaceVariant)
                        .padding(14.dp)
                ) {
                    Column {
                        Text(text = "Dung tích chứa", fontSize = 12.sp, color = TextSecondary)
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "~${(waterPercent * 20).toInt()} Lít",
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                    }
                }
            }
        }
    }
}
