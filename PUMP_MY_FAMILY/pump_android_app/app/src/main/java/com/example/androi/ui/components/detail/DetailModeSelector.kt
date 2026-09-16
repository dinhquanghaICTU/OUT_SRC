package com.example.androi.ui.components.detail

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.ui.theme.BrandOrange
import com.example.androi.ui.theme.CardBackground
import com.example.androi.ui.theme.TextPrimary

/**
 * Thanh chọn chế độ vận hành: Tự Động | Thủ Công (Đã lược bỏ Hẹn Giờ theo yêu cầu)
 */
@Composable
fun DetailModeSelector(
    modes: List<String> = listOf("Tự Động", "Thủ Công"),
    selectedModeIndex: Int,
    onModeSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        modes.forEachIndexed { index, modeTitle ->
            val isSelected = selectedModeIndex == index
            Box(
                modifier = Modifier
                    .weight(1f)
                    .height(44.dp)
                    .clip(RoundedCornerShape(22.dp))
                    .background(if (isSelected) BrandOrange else CardBackground)
                    .clickable { onModeSelected(index) },
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = modeTitle,
                    fontSize = 14.sp,
                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Medium,
                    color = if (isSelected) Color.White else TextPrimary
                )
            }
        }
    }
}
