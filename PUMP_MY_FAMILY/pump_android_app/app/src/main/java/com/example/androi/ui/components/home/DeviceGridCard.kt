package com.example.androi.ui.components.home

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
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.DeviceCardItem
import com.example.androi.data.model.DeviceIconType
import com.example.androi.ui.components.common.SmartSwitch
import com.example.androi.ui.theme.*

/**
 * Thẻ thiết bị trong lưới 2x2 (Chuẩn theo Screen 2 trong template)
 */
@Composable
fun DeviceGridCard(
    item: DeviceCardItem,
    onCardClick: () -> Unit = {},
    onToggle: (Boolean) -> Unit = {},
    modifier: Modifier = Modifier
) {
    val iconVector: ImageVector = when (item.iconType) {
        DeviceIconType.PUMP -> Icons.Rounded.WaterDrop
        DeviceIconType.WATER_TANK -> Icons.Rounded.Waves
        DeviceIconType.CHILD_LOCK -> Icons.Rounded.Lock
        DeviceIconType.SOLAR_BATTERY -> Icons.Rounded.BatteryChargingFull
        DeviceIconType.LIGHT -> Icons.Rounded.Lightbulb
        DeviceIconType.AC -> Icons.Rounded.Air
    }

    Box(
        modifier = modifier
            .clip(RoundedCornerShape(24.dp))
            .background(CardBackground)
            .clickable { onCardClick() }
            .padding(16.dp)
    ) {
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.SpaceBetween
        ) {
            // Hàng 1: Icon bên trái + SmartSwitch bên phải
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(44.dp)
                        .clip(CircleShape)
                        .background(if (item.isRunning) BrandOrange else SurfaceVariant),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = iconVector,
                        contentDescription = item.title,
                        tint = if (item.isRunning) androidx.compose.ui.graphics.Color.White else BrandOrange,
                        modifier = Modifier.size(24.dp)
                    )
                }

                SmartSwitch(
                    checked = item.isRunning,
                    onCheckedChange = onToggle
                )
            }

            Spacer(modifier = Modifier.height(18.dp))

            // Hàng 2: Tên thiết bị và trạng thái phụ
            Column {
                Text(
                    text = item.title,
                    fontSize = 15.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    text = item.subtitle,
                    fontSize = 12.sp,
                    color = if (item.isRunning) BrandOrange else TextSecondary,
                    fontWeight = if (item.isRunning) FontWeight.SemiBold else FontWeight.Normal,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
            }
        }
    }
}
