package com.example.androi.ui.components.home

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Add
import androidx.compose.material.icons.rounded.QrCode
import androidx.compose.material.icons.rounded.WaterDrop
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
import com.example.androi.ui.theme.*

/**
 * Hộp thoại Thêm / Thêm lại Máy Bơm Mới vào hệ thống
 */
@Composable
fun AddDeviceDialog(
    onDismiss: () -> Unit,
    onConfirm: (code: String, name: String) -> Unit
) {
    var deviceCode by remember { mutableStateOf("PUMP_FAMILY_01") }
    var deviceName by remember { mutableStateOf("Bơm ở sân") }
    var errorMsg by remember { mutableStateOf<String?>(null) }

    Dialog(onDismissRequest = onDismiss) {
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            shape = RoundedCornerShape(28.dp),
            colors = CardDefaults.cardColors(containerColor = CardBackground)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                // Icon tròn biểu tượng thêm thiết bị
                Box(
                    modifier = Modifier
                        .size(56.dp)
                        .clip(CircleShape)
                        .background(BrandOrange.copy(alpha = 0.12f)),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = Icons.Rounded.Add,
                        contentDescription = null,
                        tint = BrandOrange,
                        modifier = Modifier.size(28.dp)
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                Text(
                    text = "Thêm Máy Bơm Mới",
                    fontSize = 19.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )

                Spacer(modifier = Modifier.height(6.dp))

                Text(
                    text = "Nhập mã định danh ESP32 và đặt tên dễ nhớ cho máy bơm trong gia đình",
                    fontSize = 13.sp,
                    color = TextSecondary,
                    textAlign = androidx.compose.ui.text.style.TextAlign.Center,
                    lineHeight = 18.sp
                )

                Spacer(modifier = Modifier.height(20.dp))

                // Input 1: Mã Thiết Bị
                OutlinedTextField(
                    value = deviceCode,
                    onValueChange = {
                        deviceCode = it.uppercase()
                        errorMsg = null
                    },
                    label = { Text("Mã máy bơm (Device Code)") },
                    placeholder = { Text("Ví dụ: PUMP_FAMILY_01") },
                    leadingIcon = {
                        Icon(
                            imageVector = Icons.Rounded.QrCode,
                            contentDescription = null,
                            tint = BrandOrange
                        )
                    },
                    singleLine = true,
                    shape = RoundedCornerShape(16.dp),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = BrandOrange,
                        unfocusedBorderColor = Color(0xFFE2E8F0),
                        focusedLabelColor = BrandOrange
                    ),
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(modifier = Modifier.height(14.dp))

                // Input 2: Tên Thiết Bị
                OutlinedTextField(
                    value = deviceName,
                    onValueChange = {
                        deviceName = it
                        errorMsg = null
                    },
                    label = { Text("Tên gọi gợi nhớ") },
                    placeholder = { Text("Ví dụ: Bơm ở sân, Bơm tầng 3") },
                    leadingIcon = {
                        Icon(
                            imageVector = Icons.Rounded.WaterDrop,
                            contentDescription = null,
                            tint = BrandOrange
                        )
                    },
                    singleLine = true,
                    shape = RoundedCornerShape(16.dp),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = BrandOrange,
                        unfocusedBorderColor = Color(0xFFE2E8F0),
                        focusedLabelColor = BrandOrange
                    ),
                    modifier = Modifier.fillMaxWidth()
                )

                // Báo lỗi nếu để trống
                if (errorMsg != null) {
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = errorMsg!!,
                        color = Color(0xFFEF4444),
                        fontSize = 12.sp
                    )
                }

                Spacer(modifier = Modifier.height(24.dp))

                // Nút Hủy và Thêm Thiết Bị
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    OutlinedButton(
                        onClick = onDismiss,
                        modifier = Modifier
                            .weight(1f)
                            .height(48.dp),
                        shape = RoundedCornerShape(24.dp),
                        colors = ButtonDefaults.outlinedButtonColors(contentColor = TextSecondary)
                    ) {
                        Text(text = "Hủy", fontWeight = FontWeight.SemiBold)
                    }

                    Button(
                        onClick = {
                            if (deviceCode.isBlank()) {
                                errorMsg = "Vui lòng nhập mã thiết bị!"
                            } else if (deviceName.isBlank()) {
                                errorMsg = "Vui lòng đặt tên cho máy bơm!"
                            } else {
                                onConfirm(deviceCode.trim(), deviceName.trim())
                            }
                        },
                        modifier = Modifier
                            .weight(1.2f)
                            .height(48.dp),
                        shape = RoundedCornerShape(24.dp),
                        colors = ButtonDefaults.buttonColors(containerColor = BrandOrange)
                    ) {
                        Text(text = "Thêm Ngay", fontWeight = FontWeight.Bold, color = Color.White)
                    }
                }
            }
        }
    }
}
