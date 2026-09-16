package com.example.androi.ui.screens.auth

import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.androi.data.local.SessionManager
import com.example.androi.data.repository.AuthRepository
import com.example.androi.ui.components.auth.AuthInputField
import com.example.androi.ui.theme.*

/** Chế độ hiển thị xác thực */
enum class AuthMode {
        LOGIN,
        REGISTER,
        WELCOME
}

/**
 * Màn hình Đăng nhập (Login) & Đăng ký (Register) Chuẩn 100% theo template Teal/Mint:
 * - Phần trên (Top 35%): Nền Gradient Teal với đồ họa trừu tượng & tiêu đề truyền cảm hứng
 * - Phần dưới (Bottom 65%): Card trắng bo góc trên chứa form đăng nhập/đăng ký, Remember Me, nút
 * mạng xã hội Apple & Google
 */
@Composable
fun AuthScreen(
        initialMode: AuthMode = AuthMode.LOGIN,
        onAuthSuccess: () -> Unit = {},
        modifier: Modifier = Modifier
) {
        val context = LocalContext.current
        val viewModel: AuthViewModel = remember {
                val sessionManager = SessionManager.getInstance(context)
                AuthViewModel(
                        repository = AuthRepository(sessionManager = sessionManager),
                        sessionManager = sessionManager
                )
        }
        var authMode by remember { mutableStateOf(initialMode) }

        // 👈 2. Lắng nghe trạng thái từ ViewModel
        val state by viewModel.uiState.collectAsState()
        // 👈 3. Khi login thành công -> Chuyển vào màn hình chính
        LaunchedEffect(state.isLoginSuccess) {
                if (state.isLoginSuccess) {
                        onAuthSuccess()
                }
        }
        LaunchedEffect(state.isRegisterSuccess) {
                if (state.isRegisterSuccess) {
                        authMode = AuthMode.LOGIN
                }
        }
        // Form inputs state
        var fullName by remember { mutableStateOf("") }
        var email by remember { mutableStateOf("") }
        var password by remember { mutableStateOf("") }
        var confirmPassword by remember { mutableStateOf("") }
        var isPasswordVisible by remember { mutableStateOf(false) }
        var isConfirmPasswordVisible by remember { mutableStateOf(false) }
        var rememberMe by remember { mutableStateOf(true) }

        Box(
                modifier =
                        modifier.fillMaxSize()
                                .background(
                                        Brush.verticalGradient(
                                                colors = listOf(TealDark, TealPrimary)
                                        )
                                )
        ) {
                // Lớp vẽ hình minh họa mờ phía sau (Background Doodles / Gears / Waves)
                Canvas(modifier = Modifier.fillMaxSize()) {
                        drawCircle(
                                color = Color.White.copy(alpha = 0.08f),
                                radius = 120.dp.toPx(),
                                center =
                                        androidx.compose.ui.geometry.Offset(
                                                size.width * 0.85f,
                                                size.height * 0.12f
                                        )
                        )
                        drawCircle(
                                color = Color.White.copy(alpha = 0.06f),
                                radius = 70.dp.toPx(),
                                center =
                                        androidx.compose.ui.geometry.Offset(
                                                size.width * 0.15f,
                                                size.height * 0.22f
                                        )
                        )
                }

                Column(modifier = Modifier.fillMaxSize()) {
                        // ==================== PHẦN TRÊN (TOP HEADER 32%) ====================
                        Box(
                                modifier =
                                        Modifier.fillMaxWidth()
                                                .weight(0.32f)
                                                .padding(horizontal = 28.dp, vertical = 24.dp),
                                contentAlignment = Alignment.CenterStart
                        ) {
                                Column(
                                        modifier = Modifier.fillMaxWidth(),
                                        verticalArrangement = Arrangement.Center
                                ) {
                                        // Tag nhỏ tên hệ thống
                                        Row(verticalAlignment = Alignment.CenterVertically) {
                                                Icon(
                                                        imageVector = Icons.Rounded.WaterDrop,
                                                        contentDescription = null,
                                                        tint = Color.White,
                                                        modifier = Modifier.size(18.dp)
                                                )
                                                Spacer(modifier = Modifier.width(6.dp))
                                                Text(
                                                        text = "PUMP MY FAMILY",
                                                        fontSize = 13.sp,
                                                        fontWeight = FontWeight.Bold,
                                                        color = Color.White.copy(alpha = 0.9f),
                                                        letterSpacing = 1.sp
                                                )
                                        }

                                        Spacer(modifier = Modifier.height(14.dp))

                                        AnimatedContent(
                                                targetState = authMode,
                                                transitionSpec = {
                                                        fadeIn(tween(300)) togetherWith
                                                                fadeOut(tween(300))
                                                },
                                                label = "AuthHeaderTransition"
                                        ) { mode ->
                                                when (mode) {
                                                        AuthMode.LOGIN -> {
                                                                Column {
                                                                        Text(
                                                                                text =
                                                                                        "Log In to stay on\ntop of your tasks\nand projects.",
                                                                                fontSize = 24.sp,
                                                                                fontWeight =
                                                                                        FontWeight
                                                                                                .Bold,
                                                                                color = Color.White,
                                                                                lineHeight = 30.sp
                                                                        )
                                                                        Spacer(
                                                                                modifier =
                                                                                        Modifier.height(
                                                                                                6.dp
                                                                                        )
                                                                        )
                                                                        Text(
                                                                                text =
                                                                                        "Quản lý & giám sát hệ thống bơm nước gia đình",
                                                                                fontSize = 12.sp,
                                                                                color =
                                                                                        Color.White
                                                                                                .copy(
                                                                                                        alpha =
                                                                                                                0.8f
                                                                                                )
                                                                        )
                                                                }
                                                        }
                                                        AuthMode.REGISTER -> {
                                                                Column {
                                                                        Text(
                                                                                text =
                                                                                        "Create Your Account\nand Simplify Your\nWorkday",
                                                                                fontSize = 24.sp,
                                                                                fontWeight =
                                                                                        FontWeight
                                                                                                .Bold,
                                                                                color = Color.White,
                                                                                lineHeight = 30.sp
                                                                        )
                                                                        Spacer(
                                                                                modifier =
                                                                                        Modifier.height(
                                                                                                6.dp
                                                                                        )
                                                                        )
                                                                        Text(
                                                                                text =
                                                                                        "Tạo tài khoản gia đình để điều khiển & cài đặt",
                                                                                fontSize = 12.sp,
                                                                                color =
                                                                                        Color.White
                                                                                                .copy(
                                                                                                        alpha =
                                                                                                                0.8f
                                                                                                )
                                                                        )
                                                                }
                                                        }
                                                        AuthMode.WELCOME -> {
                                                                Column {
                                                                        Text(
                                                                                text =
                                                                                        "Welcome to\nSmart Pump IoT",
                                                                                fontSize = 26.sp,
                                                                                fontWeight =
                                                                                        FontWeight
                                                                                                .Bold,
                                                                                color = Color.White,
                                                                                lineHeight = 32.sp
                                                                        )
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }

                        // ==================== PHẦN DƯỚI (BOTTOM SHEET CARD 68%)
                        // ====================
                        Box(
                                modifier =
                                        Modifier.fillMaxWidth()
                                                .weight(0.68f)
                                                .clip(
                                                        RoundedCornerShape(
                                                                topStart = 34.dp,
                                                                topEnd = 34.dp
                                                        )
                                                )
                                                .background(Color.White)
                                                .padding(horizontal = 28.dp)
                        ) {
                                Column(
                                        modifier =
                                                Modifier.fillMaxSize()
                                                        .verticalScroll(rememberScrollState())
                                                        .padding(top = 28.dp, bottom = 24.dp),
                                        horizontalAlignment = Alignment.CenterHorizontally
                                ) {
                                        // Tiêu đề form & Chuyển đổi Sign In / Sign Up
                                        Text(
                                                text =
                                                        if (authMode == AuthMode.LOGIN) "Login"
                                                        else "Sign up",
                                                fontSize = 24.sp,
                                                fontWeight = FontWeight.Bold,
                                                color = TextPrimary
                                        )

                                        Spacer(modifier = Modifier.height(6.dp))

                                        // Dòng phụ đề có link chuyển đổi qua lại
                                        Row(
                                                verticalAlignment = Alignment.CenterVertically,
                                                modifier =
                                                        Modifier.clip(RoundedCornerShape(8.dp))
                                                                .clickable {
                                                                        authMode =
                                                                                if (authMode ==
                                                                                                AuthMode.LOGIN
                                                                                )
                                                                                        AuthMode.REGISTER
                                                                                else AuthMode.LOGIN
                                                                }
                                                                .padding(
                                                                        horizontal = 8.dp,
                                                                        vertical = 4.dp
                                                                )
                                        ) {
                                                Text(
                                                        text =
                                                                if (authMode == AuthMode.LOGIN)
                                                                        "Don't Have An Account? "
                                                                else "Already Have An Account? ",
                                                        fontSize = 13.sp,
                                                        color = TextSecondary
                                                )
                                                Text(
                                                        text =
                                                                if (authMode == AuthMode.LOGIN)
                                                                        "Sign Up"
                                                                else "Log In",
                                                        fontSize = 13.sp,
                                                        fontWeight = FontWeight.Bold,
                                                        color = TealPrimary
                                                )
                                        }

                                        Spacer(modifier = Modifier.height(24.dp))

                                        // ==================== CÁC Ô NHẬP LIỆU ====================
                                        if (authMode == AuthMode.LOGIN) {
                                                // Input 1: Email
                                                AuthInputField(
                                                        value = email,
                                                        onValueChange = {
                                                                email = it
                                                                viewModel.clearError() // Gõ lại thì
                                                                // xóa chữ đỏ
                                                                // báo lỗi
                                                        },
                                                        placeholder = "Nhập địa chỉ email...",
                                                        leadingIcon = Icons.Rounded.MailOutline,
                                                        keyboardType = KeyboardType.Email
                                                )
                                                Spacer(modifier = Modifier.height(14.dp))
                                                // Input 2: Password (có nút ẩn/hiện mắt mật khẩu)
                                                AuthInputField(
                                                        value = password,
                                                        onValueChange = {
                                                                password = it
                                                                viewModel.clearError()
                                                        },
                                                        placeholder = "Nhập mật khẩu...",
                                                        leadingIcon = Icons.Rounded.Lock,
                                                        isPassword = true,
                                                        isPasswordVisible = isPasswordVisible,
                                                        onTogglePasswordVisibility = {
                                                                isPasswordVisible =
                                                                        !isPasswordVisible
                                                        }
                                                )
                                        } else {

                                                // Input 3: Confirm Password
                                                AuthInputField(
                                                        value = fullName,
                                                        onValueChange = { fullName = it },
                                                        placeholder = "Full Name",
                                                        leadingIcon =
                                                                Icons.Rounded
                                                                        .PersonOutline // isPassword
                                                        // = true,
                                                        // isPasswordVisible =
                                                        //         isConfirmPasswordVisible,
                                                        // onTogglePasswordVisibility = {
                                                        //         isConfirmPasswordVisible =
                                                        //                 !isConfirmPasswordVisible
                                                        // }
                                                        )
                                                Spacer(modifier = Modifier.height(14.dp))
                                                // Màn hình SIGN UP:
                                                // Input 1: Email
                                                AuthInputField(
                                                        value = email,
                                                        onValueChange = { email = it },
                                                        placeholder = "Enter your email address",
                                                        leadingIcon = Icons.Rounded.MailOutline,
                                                        keyboardType = KeyboardType.Email
                                                )

                                                Spacer(modifier = Modifier.height(14.dp))

                                                // Input 2: Password
                                                AuthInputField(
                                                        value = password,
                                                        onValueChange = { password = it },
                                                        placeholder = "Password",
                                                        leadingIcon = Icons.Rounded.Lock,
                                                        isPassword = true,
                                                        isPasswordVisible = isPasswordVisible,
                                                        onTogglePasswordVisibility = {
                                                                isPasswordVisible =
                                                                        !isPasswordVisible
                                                        }
                                                )

                                                Spacer(modifier = Modifier.height(14.dp))

                                                // Input 3: Confirm Password
                                                AuthInputField(
                                                        value = confirmPassword,
                                                        onValueChange = { confirmPassword = it },
                                                        placeholder = "Confirm Password",
                                                        leadingIcon = Icons.Rounded.Lock,
                                                        isPassword = true,
                                                        isPasswordVisible =
                                                                isConfirmPasswordVisible,
                                                        onTogglePasswordVisibility = {
                                                                isConfirmPasswordVisible =
                                                                        !isConfirmPasswordVisible
                                                        }
                                                )
                                        }

                                        Spacer(modifier = Modifier.height(12.dp))

                                        // Hàng: Remember Me (trái) + Forgot Password (phải)
                                        Row(
                                                modifier = Modifier.fillMaxWidth(),
                                                horizontalArrangement = Arrangement.SpaceBetween,
                                                verticalAlignment = Alignment.CenterVertically
                                        ) {
                                                Row(
                                                        verticalAlignment =
                                                                Alignment.CenterVertically,
                                                        modifier =
                                                                Modifier.clickable {
                                                                        rememberMe = !rememberMe
                                                                }
                                                ) {
                                                        Checkbox(
                                                                checked = rememberMe,
                                                                onCheckedChange = {
                                                                        rememberMe = it
                                                                },
                                                                colors =
                                                                        CheckboxDefaults.colors(
                                                                                checkedColor =
                                                                                        TealPrimary,
                                                                                uncheckedColor =
                                                                                        Color(
                                                                                                0xFFCBD5E1
                                                                                        )
                                                                        ),
                                                                modifier = Modifier.size(24.dp)
                                                        )
                                                        Spacer(modifier = Modifier.width(6.dp))
                                                        Text(
                                                                text = "Remember Me",
                                                                fontSize = 12.sp,
                                                                color = TextSecondary
                                                        )
                                                }

                                                Text(
                                                        text = "Forgot Password?",
                                                        fontSize = 12.sp,
                                                        fontWeight = FontWeight.Medium,
                                                        color = TealPrimary,
                                                        modifier =
                                                                Modifier
                                                                        .clickable { /* TODO: Quên mật khẩu */
                                                                        }
                                                )
                                        }

                                        Spacer(modifier = Modifier.height(20.dp))

                                        // Nút Đăng Nhập / Đăng Ký chính màu Teal

                                        Button(
                                                onClick = {
                                                        if (authMode == AuthMode.LOGIN) {
                                                                // 👉 BẮN SỰ KIỆN GỌI API LÊN SERVER
                                                                // THẬT 180.93.113.40
                                                                viewModel.login(email, password)
                                                        } else if (authMode == AuthMode.REGISTER) {
                                                                viewModel.register(
                                                                        email,
                                                                        password,
                                                                        fullName,
                                                                        confirmPassword
                                                                )
                                                        }
                                                },
                                                enabled =
                                                        !state.isLoading, // Đang gọi mạng thì khóa
                                                // nút không cho ấn
                                                // liên tục
                                                colors =
                                                        ButtonDefaults.buttonColors(
                                                                containerColor = TealPrimary
                                                        ),
                                                shape = RoundedCornerShape(26.dp),
                                                modifier = Modifier.fillMaxWidth().height(52.dp)
                                        ) {
                                                if (state.isLoading) {
                                                        // Đang gọi API -> Hiện vòng quay tròn
                                                        CircularProgressIndicator(
                                                                color = Color.White,
                                                                modifier = Modifier.size(24.dp),
                                                                strokeWidth = 2.5.dp
                                                        )
                                                } else {
                                                        Text(
                                                                text =
                                                                        if (authMode ==
                                                                                        AuthMode.LOGIN
                                                                        )
                                                                                "Login"
                                                                        else "Sign up",
                                                                fontSize = 16.sp,
                                                                fontWeight = FontWeight.Bold,
                                                                color = Color.White
                                                        )
                                                }
                                        }

                                        // 👈 Hiện câu thông báo lỗi chữ đỏ nếu sai mật khẩu hoặc
                                        // mất mạng
                                        if (state.errorMessage != null) {
                                                Spacer(modifier = Modifier.height(12.dp))
                                                Text(
                                                        text = state.errorMessage!!,
                                                        color =
                                                                Color(
                                                                        0xFFEF4444
                                                                ), // Màu đỏ cảnh báo
                                                        fontSize = 13.sp,
                                                        fontWeight = FontWeight.Medium,
                                                        textAlign =
                                                                androidx.compose.ui.text.style
                                                                        .TextAlign.Center,
                                                        modifier = Modifier.fillMaxWidth()
                                                )
                                        }

                                        Spacer(modifier = Modifier.height(20.dp))
                                }
                        }
                }
        }
}
