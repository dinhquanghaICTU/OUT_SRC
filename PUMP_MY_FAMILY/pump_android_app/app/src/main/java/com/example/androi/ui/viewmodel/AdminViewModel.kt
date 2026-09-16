package com.example.androi.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.androi.data.model.AdminUserDto
import com.example.androi.data.model.OtaProgressResponse
import com.example.androi.data.repository.AdminRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

data class AdminScreenState(
    val users: List<AdminUserDto> = emptyList(),
    val otaProgress: OtaProgressResponse? = null,
    val isLoading: Boolean = false,
    val isOtaTriggering: Boolean = false,
    val statusMessage: String? = null,
    val errorMessage: String? = null
)

class AdminViewModel(private val repository: AdminRepository = AdminRepository()) : ViewModel() {

    private val _state = MutableStateFlow(AdminScreenState())
    val state = _state.asStateFlow()

    private var otaPollingJob: Job? = null

    init {
        loadUsers()
    }

    fun loadUsers() {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true, errorMessage = null) }
            val res = repository.getUsers()
            res.onSuccess { userList ->
                _state.update { it.copy(users = userList, isLoading = false) }
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
            }
        }
    }

    fun createUser(email: String, pass: String, fullName: String, role: String, onDone: (Boolean) -> Unit = {}) {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.createUser(email, pass, fullName, role)
            res.onSuccess {
                _state.update { it.copy(isLoading = false, statusMessage = "Đã tạo tài khoản thành công!") }
                loadUsers()
                onDone(true)
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                onDone(false)
            }
        }
    }

    fun updateUser(userId: String, fullName: String?, role: String?, pass: String?, onDone: (Boolean) -> Unit = {}) {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.updateUser(userId, fullName, role, pass)
            res.onSuccess {
                _state.update { it.copy(isLoading = false, statusMessage = "Đã cập nhật tài khoản thành công!") }
                loadUsers()
                onDone(true)
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
                onDone(false)
            }
        }
    }

    fun deleteUser(userId: String) {
        viewModelScope.launch {
            _state.update { it.copy(isLoading = true) }
            val res = repository.deleteUser(userId)
            res.onSuccess { msg ->
                _state.update { it.copy(isLoading = false, statusMessage = msg) }
                loadUsers()
            }.onFailure { err ->
                _state.update { it.copy(isLoading = false, errorMessage = err.message) }
            }
        }
    }

    fun triggerOta(target: String, version: String, url: String, size: Long, filename: String) {
        viewModelScope.launch {
            _state.update { it.copy(isOtaTriggering = true, statusMessage = "Đang kích hoạt nạp OTA...", errorMessage = null) }
            val res = repository.triggerOta(target, version, url, size, filename)
            res.onSuccess { msg ->
                _state.update { it.copy(isOtaTriggering = false, statusMessage = msg) }
                startOtaPolling()
            }.onFailure { err ->
                _state.update { it.copy(isOtaTriggering = false, errorMessage = err.message) }
            }
        }
    }

    private fun startOtaPolling() {
        otaPollingJob?.cancel()
        otaPollingJob = viewModelScope.launch {
            var retries = 30
            while (isActive && retries > 0) {
                delay(2000)
                retries--
                val res = repository.getOtaProgress()
                res.onSuccess { progress ->
                    _state.update { it.copy(otaProgress = progress) }
                    if (progress.status == "success" || progress.status == "failed") {
                        return@launch
                    }
                }
            }
        }
    }

    fun clearStatus() {
        _state.update { it.copy(statusMessage = null, errorMessage = null) }
    }

    override fun onCleared() {
        super.onCleared()
        otaPollingJob?.cancel()
    }
}
