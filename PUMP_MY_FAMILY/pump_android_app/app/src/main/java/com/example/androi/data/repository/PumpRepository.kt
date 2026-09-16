package com.example.androi.data.repository

import com.example.androi.data.api.PumpApiService
import com.example.androi.data.api.RetrofitClient
import com.example.androi.data.local.SessionManager
import com.example.androi.data.model.*
import org.json.JSONObject

private fun parseError(rawError: String?, code: Int, defaultMsg: String): String {
    return try {
        val obj = JSONObject(rawError ?: "")
        obj.optString("detail", "$defaultMsg ($code)")
    } catch (e: Exception) {
        "Lỗi máy chủ ($code)"
    }
}

class AuthRepository(
    private val apiService: PumpApiService = RetrofitClient.api,
    private val sessionManager: SessionManager? = null
) {
    suspend fun login(email: String, pass: String): Result<LoginResponse> {
        return try {
            val response = apiService.login(LoginRequest(email = email.trim(), password = pass))
            if (response.isSuccessful && response.body() != null) {
                val loginData = response.body()!!
                RetrofitClient.authToken = loginData.accessToken
                sessionManager?.saveSession(loginData.accessToken, loginData.user)
                Result.success(loginData)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Đăng nhập thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Không thể kết nối máy chủ: ${e.localizedMessage}"))
        }
    }

    suspend fun register(email: String, password: String, fullName: String): Result<RegisterResponse> {
        return try {
            val response = apiService.register(
                RegisterRequest(email = email.trim(), password = password, fullName = fullName.trim())
            )
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Đăng ký thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Không thể kết nối máy chủ: ${e.localizedMessage}"))
        }
    }

    suspend fun getProfile(): Result<UserInfo> {
        return try {
            val response = apiService.getProfile()
            if (response.isSuccessful && response.body() != null) {
                val user = response.body()!!
                sessionManager?.let { sm ->
                    sm.getToken()?.let { token -> sm.saveSession(token, user) }
                }
                Result.success(user)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Không lấy được thông tin tài khoản")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Không thể kết nối máy chủ: ${e.localizedMessage}"))
        }
    }

    fun logout() {
        sessionManager?.clearSession()
        RetrofitClient.authToken = null
    }

    fun getSavedUser(): UserInfo? = sessionManager?.getUser()
    fun isLoggedIn(): Boolean = sessionManager?.isLoggedIn() ?: false
}

class PumpRepository(private val apiService: PumpApiService = RetrofitClient.api) {

    suspend fun getDevices(): Result<DevicesResponse> {
        return try {
            val response = apiService.getDevices()
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Không tải được danh sách máy bơm")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi kết nối máy chủ: ${e.localizedMessage}"))
        }
    }

    suspend fun createDevice(deviceCode: String, name: String): Result<DeviceDto> {
        return try {
            val response = apiService.createDevice(DeviceCreateRequest(deviceCode = deviceCode.trim(), name = name.trim()))
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Thêm máy bơm thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi kết nối: ${e.localizedMessage}"))
        }
    }

    suspend fun deleteDevice(deviceId: String): Result<String> {
        return try {
            val response = apiService.deleteDevice(deviceId)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Đã xóa máy bơm thành công")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Xóa máy bơm thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi kết nối: ${e.localizedMessage}"))
        }
    }

    suspend fun controlPump(deviceId: String, action: String): Result<String> {
        return try {
            val response = apiService.controlPump(deviceId, PumpControlRequest(action = action))
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Thao tác thành công")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Lệnh điều khiển thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi gửi lệnh: ${e.localizedMessage}"))
        }
    }

    suspend fun updateTankConfig(
        deviceId: String,
        tankHeight: Float,
        offset: Float,
        minPct: Int,
        maxPct: Int
    ): Result<String> {
        return try {
            val req = TankConfigRequest(tankHeight = tankHeight, offset = offset, minPct = minPct, maxPct = maxPct)
            val response = apiService.updateTankConfig(deviceId, req)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Cấu hình téc nước thành công")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Lưu cấu hình téc nước thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi cập nhật cấu hình: ${e.localizedMessage}"))
        }
    }

    suspend fun getPumpLogs(deviceId: String, limit: Int = 50): Result<List<PumpLogDto>> {
        return try {
            val response = apiService.getPumpLogs(deviceId, limit)
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!.logs)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Không tải được nhật ký")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi tải nhật ký: ${e.localizedMessage}"))
        }
    }

    suspend fun shareDevice(deviceId: String, targetEmail: String, permission: String): Result<String> {
        return try {
            val req = DeviceShareRequest(targetEmail = targetEmail.trim(), permission = permission)
            val response = apiService.shareDevice(deviceId, req)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Chia sẻ thiết bị thành công")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Chia sẻ thiết bị thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi chia sẻ: ${e.localizedMessage}"))
        }
    }

    suspend fun revokeShare(deviceId: String, targetUserId: String): Result<String> {
        return try {
            val response = apiService.revokeShare(deviceId, targetUserId)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Đã thu hồi chia sẻ")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Thu hồi chia sẻ thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi thu hồi: ${e.localizedMessage}"))
        }
    }
}

class AdminRepository(private val apiService: PumpApiService = RetrofitClient.api) {

    suspend fun getUsers(): Result<List<AdminUserDto>> {
        return try {
            val response = apiService.getAdminUsers()
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Không tải được danh sách người dùng")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi kết nối: ${e.localizedMessage}"))
        }
    }

    suspend fun createUser(email: String, pass: String, fullName: String, role: String): Result<AdminUserDto> {
        return try {
            val req = AdminUserCreateRequest(email = email.trim(), password = pass, fullName = fullName.trim(), role = role)
            val response = apiService.createAdminUser(req)
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Tạo người dùng thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi tạo user: ${e.localizedMessage}"))
        }
    }

    suspend fun updateUser(userId: String, fullName: String?, role: String?, password: String?): Result<AdminUserDto> {
        return try {
            val req = AdminUserUpdateRequest(fullName = fullName, role = role, password = password)
            val response = apiService.updateAdminUser(userId, req)
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Cập nhật thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi cập nhật: ${e.localizedMessage}"))
        }
    }

    suspend fun deleteUser(userId: String): Result<String> {
        return try {
            val response = apiService.deleteAdminUser(userId)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Đã xóa người dùng")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Xóa người dùng thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi xóa: ${e.localizedMessage}"))
        }
    }

    suspend fun triggerOta(target: String, version: String, url: String, size: Long, filename: String, md5: String? = null): Result<String> {
        return try {
            val req = OtaTriggerRequest(target = target, version = version, url = url, size = size, filename = filename, md5 = md5)
            val response = apiService.triggerOta(req)
            if (response.isSuccessful) {
                Result.success(response.body()?.message ?: "Đã gửi lệnh nạp OTA")
            } else {
                val message = parseError(response.errorBody()?.string(), response.code(), "Bắn lệnh OTA thất bại")
                Result.failure(Exception(message))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi gửi OTA: ${e.localizedMessage}"))
        }
    }

    suspend fun getOtaProgress(): Result<OtaProgressResponse> {
        return try {
            val response = apiService.getOtaProgress()
            if (response.isSuccessful && response.body() != null) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Lỗi lấy tiến độ OTA"))
            }
        } catch (e: Exception) {
            Result.failure(Exception("Lỗi kết nối: ${e.localizedMessage}"))
        }
    }
}
