package com.example.androi.ui.screens.admin

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.model.DeviceUiState
import com.example.androi.ui.theme.*

/**
 * Màn hình Admin Quản Trị Hệ Thống (Admin & Engineer Dashboard):
 * Thiết kế chuyên nghiệp, đồng bộ với kiến trúc dự án PUMP_MY_FAMILY:
 * - Giám sát phần cứng ESP32-S3 Master & Node Cảm Biến ESP-NOW Tầng Thượng
 * - Cấu hình thông số bể nước (Calibration & NVS Flash)
 * - Quản lý kết nối MQTT Broker (HiveMQ Cloud) & FastAPI Backend
 * - Terminal Log thời gian thực từ ESP32 & Nút can thiệp khẩn cấp (Reboot, OTA)
 */
@Composable
fun AdminDashboardScreen(
    deviceState: DeviceUiState = DeviceUiState(),
    logs: List<com.example.androi.data.model.PumpLogDto> = emptyList(),
    onSaveTankConfig: (Float, Float, Int, Int) -> Unit = { _, _, _, _ -> },
    onTriggerOta: (String) -> Unit = {},
    onBackClick: () -> Unit = {},
    onLogout: () -> Unit = {},
    modifier: Modifier = Modifier
) {
    var selectedSection by remember { mutableStateOf(0) }
    val sections = listOf("Phần Cứng & Chip", "Hiệu Chuẩn Bể", "Mạng & MQTT", "Nhật Ký Bơm")

    // State hiệu chuẩn bể
    var tankHeightCm by remember(deviceState.tankHeightCm) { mutableStateOf(deviceState.tankHeightCm.toInt().toString()) }
    var blindZoneCm by remember(deviceState.sensorOffsetCm) { mutableStateOf(deviceState.sensorOffsetCm.toInt().toString()) }
    var tankCapacityLit by remember { mutableStateOf("2000") }
    var isCalibSaved by remember { mutableStateOf(false) }

    LazyColumn(
        modifier = modifier
            .fillMaxSize()
            .background(ScreenBackground),
        contentPadding = PaddingValues(bottom = 120.dp)
    ) {
        // 1. Header Quản Trị Viên
        item {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp, vertical = 16.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        // Badge Admin màu cam
                        Box(
                            modifier = Modifier
                                .clip(RoundedCornerShape(8.dp))
                                .background(BrandOrange.copy(alpha = 0.15f))
                                .padding(horizontal = 10.dp, vertical = 4.dp)
                        ) {
                            Text(
                                text = "SYSTEM ADMINISTRATOR",
                                fontSize = 11.sp,
                                fontWeight = FontWeight.Bold,
                                color = BrandOrange,
                                letterSpacing = 0.5.sp
                            )
                        }

                        Spacer(modifier = Modifier.height(6.dp))

                        Text(
                            text = "Trung Tâm Quản Trị",
                            fontSize = 24.sp,
                            fontWeight = FontWeight.Black,
                            color = TextPrimary
                        )

                        Text(
                            text = "Kỹ sư điều hành: QUANG HÀ ICTU",
                            fontSize = 13.sp,
                            color = TextSecondary
                        )
                    }

                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        // Nút Đăng xuất
                        Box(
                            modifier = Modifier
                                .size(42.dp)
                                .clip(CircleShape)
                                .background(SurfaceVariant)
                                .clickable { onLogout() },
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(
                                imageVector = Icons.Rounded.Logout,
                                contentDescription = "Đăng xuất",
                                tint = TextSecondary,
                                modifier = Modifier.size(20.dp)
                            )
                        }

                        // Avatar Admin / Shield Icon
                        Box(
                            modifier = Modifier
                                .size(46.dp)
                                .clip(CircleShape)
                                .background(BrandOrange),
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(
                                imageVector = Icons.Rounded.AdminPanelSettings,
                                contentDescription = "Admin",
                                tint = Color.White,
                                modifier = Modifier.size(26.dp)
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Thanh trạng thái kết nối Cloud / Network Status Pills
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    StatusBadge(
                        label = "FastAPI",
                        status = "24ms",
                        isOk = true,
                        modifier = Modifier.weight(1f)
                    )
                    StatusBadge(
                        label = "HiveMQ",
                        status = "TLS 8883",
                        isOk = true,
                        modifier = Modifier.weight(1f)
                    )
                    StatusBadge(
                        label = "ESP-NOW",
                        status = "2 Nodes",
                        isOk = true,
                        modifier = Modifier.weight(1f)
                    )
                }
            }
        }

        // 2. Thanh cuộn chọn phân hệ quản trị (Horizontal Scrollable Pills)
        item {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(rememberScrollState())
                    .padding(horizontal = 24.dp),
                horizontalArrangement = Arrangement.spacedBy(10.dp)
            )
            {
//                sections.forEachIndexed { index, title ->
//                    val isSelected = selectedSection == index
//                    Box(
//                        modifier = Modifier
//                            .clip(RoundedCornerShape(20.dp))
//                            .background(if (isSelected) BrandOrange else CardBackground)
//                            .clickable { selectedSection = index }
//                            .padding(horizontal = 18.dp, vertical = 9.dp),
//                        contentAlignment = Alignment.Center
//                    ) {
//                        Text(
//                            text = title,
//                            fontSize = 13.sp,
//                            fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Medium,
//                            color = if (isSelected) Color.White else TextPrimary
//                        )
//                    }
//                }
            }
            Spacer(modifier = Modifier.height(18.dp))
        }

        // 3. Nội dung theo phân hệ được chọn
//        when (selectedSection) {
//            0 -> {
//                // PHÂN HỆ 1: GIÁM SÁT PHẦN CỨNG & TELEMETRY
//                item {
//                    Column(
//                        modifier = Modifier.padding(horizontal = 24.dp),
//                        verticalArrangement = Arrangement.spacedBy(16.dp)
//                    ) {
//                        // Card 1: ESP32-S3 Master
//                        AdminCard(
//                            title = "Tủ Bơm Master (ESP32-S3)",
//                            subtitle = "Xtensa LX7 Dual-Core 240MHz • FreeRTOS",
//                            icon = Icons.Rounded.Memory
//                        ) {
//                            HardwareMetricRow("Bộ nhớ Internal SRAM", "512 KB (Free 384 KB)")
//                            HardwareMetricRow("Bộ nhớ Octal PSRAM", "8 MB N8R8 (Used 1.4 MB)")
//                            HardwareMetricRow("Bộ nhớ Flash SPI", "16 MB Octal Flash")
//                            HardwareMetricRow("Nhiệt độ chip ESP32", "41.8 °C (Bình thường)")
//                            HardwareMetricRow("Thời gian chạy (Uptime)", "14 ngày 08 giờ")
//                            HardwareMetricRow("Trạng thái Relay Động Cơ", "Sẵn sàng • Chịu tải 10A")
//                            HardwareMetricRow("Điện áp lưới AC 220V", "226V - 50Hz (Ổn định)")
//
//                            Spacer(modifier = Modifier.height(14.dp))
//
//                            Row(
//                                modifier = Modifier.fillMaxWidth(),
//                                horizontalArrangement = Arrangement.spacedBy(10.dp)
//                            ) {
//                                Button(
//                                    onClick = { /* TODO: Reboot ESP32 */ },
//                                    colors = ButtonDefaults.buttonColors(containerColor = SurfaceVariant),
//                                    shape = RoundedCornerShape(14.dp),
//                                    modifier = Modifier.weight(1f)
//                                ) {
//                                    Icon(Icons.Rounded.RestartAlt, contentDescription = null, tint = TextPrimary, modifier = Modifier.size(16.dp))
//                                    Spacer(modifier = Modifier.width(6.dp))
//                                    Text("Reboot S3", fontSize = 12.sp, color = TextPrimary, fontWeight = FontWeight.SemiBold)
//                                }
//
//                                Button(
//                                    onClick = { /* TODO: Check OTA */ },
//                                    colors = ButtonDefaults.buttonColors(containerColor = BrandOrange),
//                                    shape = RoundedCornerShape(14.dp),
//                                    modifier = Modifier.weight(1f)
//                                ) {
//                                    Icon(Icons.Rounded.SystemUpdate, contentDescription = null, tint = Color.White, modifier = Modifier.size(16.dp))
//                                    Spacer(modifier = Modifier.width(6.dp))
//                                    Text("Check OTA", fontSize = 12.sp, color = Color.White, fontWeight = FontWeight.SemiBold)
//                                }
//                            }
//                        }
//
//                        // Card 2: Node Cảm Biến Bể Nước (ESP-NOW Tầng Thượng)
//                        AdminCard(
//                            title = "Node Cảm Biến Tầng Thượng",
//                            subtitle = "Giao thức ESP-NOW Peer-to-Peer • Năng lượng Solar",
//                            icon = Icons.Rounded.Sensors
//                        ) {
//                            HardwareMetricRow("Địa chỉ MAC Node", "34:85:18:9B:2A:40")
//                            HardwareMetricRow("Cảm biến mặt nước", "JSN-SR04T IP67 Waterproof")
//                            HardwareMetricRow("Tín hiệu ESP-NOW RSSI", "-62 dBm (Kết nối rất tốt)")
//                            HardwareMetricRow("Pin Lithium 18650", "4.15V (97%)")
//                            HardwareMetricRow("Tấm thu năng lượng Solar", "Đang nạp +380mA (Nắng)")
//                            HardwareMetricRow("Chu kỳ đo & gửi gói tin", "30s / lần (Deep Sleep)")
//                        }
//                    }
//                }
//            }
//
//            1 -> {
//                // PHÂN HỆ 2: HIỆU CHUẨN THÔNG SỐ BỂ NƯỚC (CALIBRATION)
//                item {
//                    Column(
//                        modifier = Modifier.padding(horizontal = 24.dp),
//                        verticalArrangement = Arrangement.spacedBy(16.dp)
//                    ) {
//                        AdminCard(
//                            title = "Hiệu Chuẩn Kích Thước Bể",
//                            subtitle = "Lưu trực tiếp vào bộ nhớ Non-Volatile Storage (NVS)",
//                            icon = Icons.Rounded.Tune
//                        ) {
//                            Text(
//                                text = "Thiết lập kích thước thực tế của bể inox để vi điều khiển ESP32 tính toán chính xác % và thể tích lít nước còn lại.",
//                                fontSize = 13.sp,
//                                color = TextSecondary,
//                                lineHeight = 19.sp
//                            )
//
//                            Spacer(modifier = Modifier.height(14.dp))
//
//                            OutlinedTextField(
//                                value = tankHeightCm,
//                                onValueChange = { tankHeightCm = it },
//                                label = { Text("Chiều cao lòng bể nước (cm)") },
//                                modifier = Modifier.fillMaxWidth(),
//                                shape = RoundedCornerShape(16.dp),
//                                singleLine = true
//                            )
//
//                            Spacer(modifier = Modifier.height(10.dp))
//
//                            OutlinedTextField(
//                                value = blindZoneCm,
//                                onValueChange = { blindZoneCm = it },
//                                label = { Text("Khoảng cách mù cảm biến (cm)") },
//                                modifier = Modifier.fillMaxWidth(),
//                                shape = RoundedCornerShape(16.dp),
//                                singleLine = true
//                            )
//
//                            Spacer(modifier = Modifier.height(10.dp))
//
//                            OutlinedTextField(
//                                value = tankCapacityLit,
//                                onValueChange = { tankCapacityLit = it },
//                                label = { Text("Dung tích tối đa của bể (Lít)") },
//                                modifier = Modifier.fillMaxWidth(),
//                                shape = RoundedCornerShape(16.dp),
//                                singleLine = true
//                            )
//
//                            Spacer(modifier = Modifier.height(16.dp))
//
//                            Button(
//                                onClick = {
//                                    onSaveTankConfig(
//                                        tankHeightCm.toFloatOrNull() ?: 150f,
//                                        blindZoneCm.toFloatOrNull() ?: 20f,
//                                        deviceState.minWaterPercent,
//                                        deviceState.maxWaterPercent
//                                    )
//                                    isCalibSaved = true
//                                },
//                                colors = ButtonDefaults.buttonColors(containerColor = BrandOrange),
//                                shape = RoundedCornerShape(16.dp),
//                                modifier = Modifier.fillMaxWidth()
//                            ) {
//                                Icon(Icons.Rounded.Save, contentDescription = null, tint = Color.White)
//                                Spacer(modifier = Modifier.width(8.dp))
//                                Text(
//                                    text = if (isCalibSaved) "✓ Đã Ghi Vào NVS Flash" else "Lưu Cấu Hình Vào Flash ESP32",
//                                    fontSize = 14.sp,
//                                    fontWeight = FontWeight.Bold,
//                                    color = Color.White
//                                )
//                            }
//                        }
//                    }
//                }
//            }
//
//            2 -> {
//                // PHÂN HỆ 3: CẤU HÌNH MẠNG & MQTT BROKER
//                item {
//                    Column(
//                        modifier = Modifier.padding(horizontal = 24.dp),
//                        verticalArrangement = Arrangement.spacedBy(16.dp)
//                    ) {
//                        AdminCard(
//                            title = "Thông Số Kết Nối Broker",
//                            subtitle = "HiveMQ Cloud & FastAPI IoT Gateway",
//                            icon = Icons.Rounded.CloudDone
//                        ) {
//                            HardwareMetricRow("MQTT Broker URL", "b492xxx.s1.eu.hivemq.cloud")
//                            HardwareMetricRow("Cổng kết nối TLS", "8883 (Mã hóa SSL/TLS)")
//                            HardwareMetricRow("Client ID", deviceState.deviceCode.ifBlank { "ESP32S3_PUMP_MASTER_01" })
//                            HardwareMetricRow("Topic Lệnh (Pub)", "family_pump/cmd/relay")
//                            HardwareMetricRow("Topic Trạng Thái (Sub)", "family_pump/telemetry/state")
//                            HardwareMetricRow("FastAPI REST API", "http://180.93.113.40")
//                            HardwareMetricRow("WebSocket Realtime", "ws://180.93.113.40/ws")
//
//                            Spacer(modifier = Modifier.height(14.dp))
//
//                            Button(
//                                onClick = { /* Ping Broker */ },
//                                colors = ButtonDefaults.buttonColors(containerColor = SurfaceVariant),
//                                shape = RoundedCornerShape(16.dp),
//                                modifier = Modifier.fillMaxWidth()
//                            ) {
//                                Icon(Icons.Rounded.NetworkCheck, contentDescription = null, tint = BrandOrange)
//                                Spacer(modifier = Modifier.width(8.dp))
//                                Text("Kiểm Tra Độ Trễ (Ping Test)", color = TextPrimary, fontWeight = FontWeight.SemiBold, fontSize = 13.sp)
//                            }
//                        }
//                    }
//                }
//            }
//
//            3 -> {
//                // PHÂN HỆ 4: TERMINAL LOG THỜI GIAN THỰC (SERIAL CONSOLE)
//                item {
//                    Column(
//                        modifier = Modifier.padding(horizontal = 24.dp),
//                        verticalArrangement = Arrangement.spacedBy(14.dp)
//                    ) {
//                        AdminCard(
//                            title = "Nhật Ký Hệ Thống (Console Log)",
//                            subtitle = "Luồng dữ liệu thời gian thực từ ESP32 và Server",
//                            icon = Icons.Rounded.Terminal
//                        ) {
//                            // Khung hiển thị log phong cách Terminal đen
//                            Box(
//                                modifier = Modifier
//                                    .fillMaxWidth()
//                                    .height(260.dp)
//                                    .clip(RoundedCornerShape(16.dp))
//                                    .background(Color(0xFF1E242B))
//                                    .padding(14.dp)
//                            ) {
//                                Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
//                                    if (logs.isNotEmpty()) {
//                                        logs.take(8).forEach { log ->
//                                            val logColor = when {
//                                                log.action.contains("ON", ignoreCase = true) -> Color(0xFF10B981)
//                                                log.action.contains("OFF", ignoreCase = true) -> Color(0xFFF87171)
//                                                else -> Color(0xFF38BDF8)
//                                            }
//                                            Text(
//                                                text = "[${log.createdAt.takeLast(8).ifBlank { "Live" }}] ${log.action} | Nước: ${log.waterLevel ?: 0f}%",
//                                                fontSize = 11.sp,
//                                                color = logColor,
//                                                fontFamily = FontFamily.Monospace
//                                            )
//                                        }
//                                    } else {
//                                        Text("[09:44:02] [ESP32-S3] [I] Wi-Fi Connected. Target: 180.93.113.40", fontSize = 11.sp, color = Color(0xFF10B981), fontFamily = FontFamily.Monospace)
//                                        Text("[09:44:03] [ESP32-S3] [I] MQTT Connected to HiveMQ Cloud (TLS 8883)", fontSize = 11.sp, color = Color(0xFF38BDF8), fontFamily = FontFamily.Monospace)
//                                        Text("[09:44:05] [ESP-NOW] [I] Registered Tank Node: 34:85:18:9B:2A:40", fontSize = 11.sp, color = Color(0xFFFBBF24), fontFamily = FontFamily.Monospace)
//                                        Text("[09:44:35] [ESP-NOW] [D] Packet: dist=${deviceState.distanceCm}cm, vbat=${deviceState.batteryVoltage}V", fontSize = 11.sp, color = Color.White, fontFamily = FontFamily.Monospace)
//                                        Text("[09:44:35] [CALC] Water: ${deviceState.waterPercent}%", fontSize = 11.sp, color = Color(0xFF34D399), fontFamily = FontFamily.Monospace)
//                                        Text("[09:45:00] [PUMP_CTRL] Auto Mode=${deviceState.isAutoMode}, Pump=${deviceState.isPumpRunning}", fontSize = 11.sp, color = Color(0xFF94A3B8), fontFamily = FontFamily.Monospace)
//                                    }
//                                }
//                            }
//
//                            Spacer(modifier = Modifier.height(10.dp))
//
//                            Row(
//                                modifier = Modifier.fillMaxWidth(),
//                                horizontalArrangement = Arrangement.SpaceBetween,
//                                verticalAlignment = Alignment.CenterVertically
//                            ) {
//                                Text("Trạng thái: Live Stream", fontSize = 12.sp, color = Color(0xFF10B981), fontWeight = FontWeight.Medium)
//                                Text("Tự cuộn theo log", fontSize = 12.sp, color = TextSecondary)
//                            }
//                        }
//                    }
//                }
//            }
//        }
    }
}

/**
 * Card bao bọc một khu vực thông số quản trị
 */
@Composable
private fun AdminCard(
    title: String,
    subtitle: String,
    icon: ImageVector,
    content: @Composable ColumnScope.() -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(26.dp))
            .background(CardBackground)
            .padding(20.dp)
    ) {
        Column {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Box(
                    modifier = Modifier
                        .size(42.dp)
                        .clip(CircleShape)
                        .background(SurfaceVariant),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = icon,
                        contentDescription = null,
                        tint = BrandOrange,
                        modifier = Modifier.size(22.dp)
                    )
                }

                Column {
                    Text(
                        text = title,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.Bold,
                        color = TextPrimary
                    )
                    Text(
                        text = subtitle,
                        fontSize = 12.sp,
                        color = TextSecondary
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))
            content()
        }
    }
}

/**
 * Hàng hiển thị một thông số kỹ thuật (Label & Value)
 */
@Composable
private fun HardwareMetricRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 5.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = label,
            fontSize = 13.sp,
            color = TextSecondary,
            modifier = Modifier.weight(1f)
        )
        Text(
            text = value,
            fontSize = 13.sp,
            fontWeight = FontWeight.SemiBold,
            color = TextPrimary,
            textAlign = TextAlign.End,
            modifier = Modifier.weight(1.3f)
        )
    }
}

/**
 * Huy hiệu trạng thái kết nối
 */
@Composable
private fun StatusBadge(
    label: String,
    status: String,
    isOk: Boolean,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .clip(RoundedCornerShape(16.dp))
            .background(CardBackground)
            .padding(vertical = 10.dp, horizontal = 10.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Box(
                    modifier = Modifier
                        .size(6.dp)
                        .clip(CircleShape)
                        .background(if (isOk) Color(0xFF10B981) else Color(0xFFEF4444))
                )
                Spacer(modifier = Modifier.width(5.dp))
                Text(
                    text = label,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Medium,
                    color = TextSecondary
                )
            }
            Spacer(modifier = Modifier.height(2.dp))
            Text(
                text = status,
                fontSize = 12.sp,
                fontWeight = FontWeight.Bold,
                color = TextPrimary
            )
        }
    }
}
