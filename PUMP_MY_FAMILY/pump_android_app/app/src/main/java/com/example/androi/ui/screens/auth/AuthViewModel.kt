package com.example.androi.ui.screens.auth

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.androi.data.local.SessionManager
import com.example.androi.data.model.UserInfo
import com.example.androi.data.repository.AuthRepository
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

// 1. Trạng thái giao diện mà màn hình Auth cần quan sát
data class AuthUiState(
    val isLoading: Boolean = false,
    val isLoginSuccess: Boolean = false,
    val isRegisterSuccess: Boolean = false,
    val currentUser: UserInfo? = null,
    val errorMessage: String? = null
)

// 2. ViewModel xử lý logic
class AuthViewModel(
    private val repository: AuthRepository = AuthRepository(),
    private val sessionManager: SessionManager? = null
) : ViewModel() {

    // Luồng dữ liệu trạng thái (StateFlow)
    private val _uiState = MutableStateFlow(AuthUiState())
    val uiState = _uiState.asStateFlow()

    init {
        // Tự động kiểm tra trạng thái đăng nhập đã lưu
        val savedUser = sessionManager?.getUser()
        if (savedUser != null && sessionManager.isLoggedIn()) {
            _uiState.update {
                it.copy(
                    isLoginSuccess = true,
                    currentUser = savedUser
                )
            }
            // Gọi refresh profile ngầm
            refreshProfile()
        }
    }

    fun refreshProfile() {
        viewModelScope.launch {
            repository.getProfile().onSuccess { user ->
                _uiState.update { it.copy(currentUser = user) }
            }
        }
    }

    /** Xử lý ĐĂNG NHẬP */
    fun login(email: String, pass: String) {
        if (email.isBlank() || pass.isBlank()) {
            _uiState.update { it.copy(errorMessage = "Email và mật khẩu không được để trống!") }
            return
        }

        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true, errorMessage = null) }

            val result = repository.login(email, pass)

            result.onSuccess { data ->
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        isLoginSuccess = true,
                        currentUser = data.user
                    )
                }
            }.onFailure { error ->
                android.util.Log.e("AUTH_DEBUG", "Lỗi đăng nhập: ${error.message}")
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        errorMessage = error.message ?: "Đăng nhập thất bại!"
                    )
                }
            }
        }
    }

    /** Xử lý ĐĂNG KÝ */
    fun register(email: String, password: String, fullName: String, confirmPass: String) {
        if (email.isBlank() || password.isBlank() || fullName.isBlank() || confirmPass.isBlank()) {
            _uiState.update { it.copy(errorMessage = "Vui lòng điền đầy đủ các trường thông tin!") }
            return
        }
        if (password != confirmPass) {
            _uiState.update { it.copy(errorMessage = "Mật khẩu và xác nhận mật khẩu không khớp!") }
            return
        }
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true, errorMessage = null) }
            val result = repository.register(email, password, fullName)
            result.onSuccess {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        isRegisterSuccess = true
                    )
                }
            }.onFailure { error ->
                android.util.Log.e("AUTH_DEBUG", "Lỗi đăng ký: ${error.message}")
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        errorMessage = error.message ?: "Đăng ký thất bại!"
                    )
                }
            }
        }
    }

    fun logout() {
        repository.logout()
        _uiState.update {
            AuthUiState(
                isLoginSuccess = false,
                currentUser = null
            )
        }
    }

    /** Xóa thông báo lỗi khi người dùng bắt đầu gõ lại */
    fun clearError() {
        _uiState.update { it.copy(errorMessage = null) }
    }
}
