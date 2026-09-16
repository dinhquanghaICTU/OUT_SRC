package com.example.androi.ui.components.common

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.CheckCircle
import androidx.compose.material.icons.rounded.Email
import androidx.compose.material.icons.rounded.RadioButtonUnchecked
import androidx.compose.material.icons.rounded.Share
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.example.androi.ui.theme.BrandOrange
import com.example.androi.ui.theme.CardBackground
import com.example.androi.ui.theme.TextPrimary
import com.example.androi.ui.theme.TextSecondary

/**
 * Dialog chia sẻ quyền sử dụng máy bơm với người thân trong gia đình
 * Đồng bộ chuẩn API backend: POST /api/devices/{device_id}/share
 */
@Composable
fun ShareDeviceDialog(
    deviceName: String = "Máy Bơm Gia Đình",
    onDismiss: () -> Unit,
    onShare: (email: String, permission: String) -> Unit
) {
    var email by remember { mutableStateOf("") }
    var selectedPermission by remember { mutableStateOf("CAN_CONTROL") } // "CAN_CONTROL" hoặc "VIEW_ONLY"
    var errorText by remember { mutableStateOf<String?>(null) }

    Dialog(onDismissRequest = onDismiss) {
        Card(
            shape = RoundedCornerShape(28.dp),
            colors = CardDefaults.cardColors(containerColor = CardBackground),
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 4.dp),
            elevation = CardDefaults.cardElevation(defaultElevation = 12.dp)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(24.dp)
            ) {
                // Tiêu đề
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(10.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .size(40.dp)
                            .clip(RoundedCornerShape(12.dp))
                            .background(BrandOrange.copy(alpha = 0.15f)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Rounded.Share,
                            contentDescription = null,
                            tint = BrandOrange,
                            modifier = Modifier.size(20.dp)
                        )
                    }
                    Column {
                        Text(
                            text = "Chia Sẻ Thiết Bị",
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold,
                            color = TextPrimary
                        )
                        Text(
                            text = deviceName,
                            fontSize = 12.sp,
                            color = TextSecondary
                        )
                    }
                }

                Spacer(modifier = Modifier.height(18.dp))

                // Ô nhập Email
                Text(
                    text = "Email Người Thân",
                    fontSize = 13.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = TextPrimary
                )
                Spacer(modifier = Modifier.height(6.dp))

                OutlinedTextField(
                    value = email,
                    onValueChange = {
                        email = it
                        errorText = null
                    },
                    placeholder = { Text("vd: vo_yeu@gmail.com", fontSize = 14.sp) },
                    leadingIcon = {
                        Icon(
                            imageVector = Icons.Rounded.Email,
                            contentDescription = null,
                            tint = TextSecondary,
                            modifier = Modifier.size(20.dp)
                        )
                    },
                    singleLine = true,
                    shape = RoundedCornerShape(16.dp),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = BrandOrange,
                        unfocusedBorderColor = Color(0xFFE2E8F0)
                    ),
                    modifier = Modifier.fillMaxWidth()
                )

                if (errorText != null) {
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = errorText!!,
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.error
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Chọn phân quyền
                Text(
                    text = "Cấp Quyền Sử Dụng",
                    fontSize = 13.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = TextPrimary
                )
                Spacer(modifier = Modifier.height(8.dp))

                // Quyền 1: Toàn quyền
                PermissionOption(
                    title = "Toàn Quyền Điều Khiển (CAN_CONTROL)",
                    desc = "Bật/Tắt bơm & cấu hình thông số bể",
                    isSelected = selectedPermission == "CAN_CONTROL",
                    onClick = { selectedPermission = "CAN_CONTROL" }
                )

                Spacer(modifier = Modifier.height(8.dp))

                // Quyền 2: Chỉ xem
                PermissionOption(
                    title = "Chỉ Xem Trạng Thái (VIEW_ONLY)",
                    desc = "Theo dõi % nước, pin & lịch sử bơm",
                    isSelected = selectedPermission == "VIEW_ONLY",
                    onClick = { selectedPermission = "VIEW_ONLY" }
                )

                Spacer(modifier = Modifier.height(24.dp))

                // Các nút hành động
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(10.dp)
                ) {
                    OutlinedButton(
                        onClick = onDismiss,
                        shape = RoundedCornerShape(14.dp),
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Hủy", color = TextSecondary)
                    }

                    Button(
                        onClick = {
                            if (email.isBlank() || !email.contains("@")) {
                                errorText = "Vui lòng nhập đúng địa chỉ email!"
                            } else {
                                onShare(email.trim(), selectedPermission)
                                onDismiss()
                            }
                        },
                        shape = RoundedCornerShape(14.dp),
                        colors = ButtonDefaults.buttonColors(containerColor = BrandOrange),
                        modifier = Modifier.weight(1.3f)
                    ) {
                        Text("Chia Sẻ", fontWeight = FontWeight.Bold, color = Color.White)
                    }
                }
            }
        }
    }
}

@Composable
private fun PermissionOption(
    title: String,
    desc: String,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(14.dp))
            .background(if (isSelected) BrandOrange.copy(alpha = 0.08f) else Color(0xFFF8FAFC))
            .clickable { onClick() }
            .padding(horizontal = 12.dp, vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Icon(
            imageVector = if (isSelected) Icons.Rounded.CheckCircle else Icons.Rounded.RadioButtonUnchecked,
            contentDescription = null,
            tint = if (isSelected) BrandOrange else TextSecondary,
            modifier = Modifier.size(20.dp)
        )
        Column {
            Text(
                text = title,
                fontSize = 13.sp,
                fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Medium,
                color = if (isSelected) BrandOrange else TextPrimary
            )
            Text(
                text = desc,
                fontSize = 11.sp,
                color = TextSecondary
            )
        }
    }
}
