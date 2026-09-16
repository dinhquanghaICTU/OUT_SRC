package com.example.androi.ui.screens.overview

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Add
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
import com.example.androi.data.model.DeviceUiState
import com.example.androi.ui.components.home.FamilyPhotoSlider
import com.example.androi.ui.components.home.HeaderGreeting
import com.example.androi.ui.components.home.SingleDeviceCard
import com.example.androi.ui.theme.*

/**
 * Màn hình Trang Chủ (Overview Screen):
 * - Phía trên: Lời chào + Avatar (HeaderGreeting) & Slide ảnh gia đình tự chạy (FamilyPhotoSlider)
 * - Phía dưới: Ban đầu hiển thị 1 ITEM thiết bị duy nhất (SingleDeviceCard)
 * - Khi click vào card thiết bị sẽ chuyển sang Màn hình Chi tiết
 */
@Composable
fun OverviewScreen(
        deviceState: DeviceUiState = DeviceUiState(),
        deviceCount: Int = 1,
        unreadCount: Int = 0,
        onTogglePump: (Boolean) -> Unit = {},
        onDeviceClick: (String) -> Unit = {},
        onNotificationClick: () -> Unit = {},
        onAddDeviceClick: () -> Unit = {},
        modifier: Modifier = Modifier
) {
    LazyColumn(
            modifier = modifier.fillMaxSize().background(ScreenBackground),
            contentPadding = PaddingValues(bottom = 120.dp)
    ) {
        // 1. Header Lời chào & Avatar
        item {
            HeaderGreeting(
                    userName = "Hà",
                    greeting = "Chào gia đình nhỏ",
                    unreadCount = unreadCount,
                    onNotificationClick = onNotificationClick
            )
        }

        // 2. Slide ảnh gia đình (FamilyPhotoSlider)
        item {
            Spacer(modifier = Modifier.height(6.dp))
            FamilyPhotoSlider(
                    modifier = Modifier.fillMaxWidth().height(290.dp)
            )
            Spacer(modifier = Modifier.height(20.dp))
        }

        // 3. Tiêu đề danh sách thiết bị + Nút Thêm (+)
        item {
            Row(
                    modifier = Modifier.fillMaxWidth().padding(horizontal = 24.dp),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
            ) {
                Column {
                    Text(
                            text = "Thiết Bị Của Bạn",
                            fontSize = 20.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                    )
                    Text(
                        text = if (deviceCount > 0) "$deviceCount thiết bị đang kết nối" else "Chưa có thiết bị (Bấm + để thêm)",
                        fontSize = 13.sp,
                        color = TextSecondary
                    )
                }

                Box(
                        modifier =
                                Modifier.size(40.dp)
                                        .clip(CircleShape)
                                        .background(BrandOrange)
                                        .clickable { onAddDeviceClick() },
                        contentAlignment = Alignment.Center
                ) {
                    Icon(
                            imageVector = Icons.Rounded.Add,
                            contentDescription = "Add device",
                            tint = Color.White,
                            modifier = Modifier.size(22.dp)
                    )
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
        }

        // 4. Card Thiết bị hoặc Thông báo Chưa có thiết bị
        item {
            if (deviceCount > 0 && deviceState.id.isNotBlank()) {
                SingleDeviceCard(
                        deviceState = deviceState,
                        onTogglePump = onTogglePump,
                        onCardClick = { onDeviceClick(deviceState.id) }
                )
            } else {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 24.dp)
                        .clip(androidx.compose.foundation.shape.RoundedCornerShape(24.dp))
                        .background(CardBackground)
                        .clickable { onAddDeviceClick() }
                        .padding(32.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(
                            imageVector = Icons.Rounded.Add,
                            contentDescription = null,
                            tint = BrandOrange,
                            modifier = Modifier.size(36.dp)
                        )
                        Spacer(modifier = Modifier.height(10.dp))
                        Text(
                            text = "Chưa Có Máy Bơm Nào",
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "Bấm vào đây hoặc nút (+) để thêm máy bơm",
                            fontSize = 13.sp,
                            color = TextSecondary
                        )
                    }
                }
            }
        }
    }
}
