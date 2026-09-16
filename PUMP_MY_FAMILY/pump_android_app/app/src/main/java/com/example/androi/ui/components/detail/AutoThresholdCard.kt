package com.example.androi.ui.components.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.ui.theme.*

/**
 * Card Cài Đặt Ngưỡng Tự Động (Auto Threshold Settings)
 */
@Composable
fun AutoThresholdCard(
    onThresholdPercent: Int = 30,
    offThresholdPercent: Int = 95,
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
            Text(
                text = "Cài Đặt Ngưỡng Tự Động",
                fontSize = 17.sp,
                fontWeight = FontWeight.Bold,
                color = TextPrimary
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = "Bơm sẽ tự kích hoạt khi nước cạn và tự ngắt khi đầy",
                fontSize = 13.sp,
                color = TextSecondary
            )

            Spacer(modifier = Modifier.height(16.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(14.dp)
            ) {
                // Ngưỡng Bật khi dưới X%
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .clip(RoundedCornerShape(18.dp))
                        .background(SurfaceVariant)
                        .padding(14.dp)
                ) {
                    Column {
                        Text(text = "Tự BẬT khi dưới", fontSize = 12.sp, color = TextSecondary)
                        Spacer(modifier = Modifier.height(6.dp))
                        Text(
                            text = "$onThresholdPercent%",
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold,
                            color = BrandOrange
                        )
                    }
                }

                // Ngưỡng Tắt khi đầy Y%
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .clip(RoundedCornerShape(18.dp))
                        .background(SurfaceVariant)
                        .padding(14.dp)
                ) {
                    Column {
                        Text(text = "Tự TẮT khi đạt", fontSize = 12.sp, color = TextSecondary)
                        Spacer(modifier = Modifier.height(6.dp))
                        Text(
                            text = "$offThresholdPercent%",
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                    }
                }
            }
        }
    }
}
