package com.example.androi.data.api

import com.example.androi.data.model.*
import retrofit2.Response
import retrofit2.http.*

interface PumpApiService {

    // ==================== AUTH ====================
    @POST("api/auth/login")
    suspend fun login(@Body request: LoginRequest): Response<LoginResponse>

    @POST("api/auth/register")
    suspend fun register(@Body request: RegisterRequest): Response<RegisterResponse>

    @GET("api/auth/me")
    suspend fun getProfile(): Response<UserInfo>

    // ==================== DEVICES ====================
    @GET("api/devices/")
    suspend fun getDevices(): Response<DevicesResponse>

    @POST("api/devices/")
    suspend fun createDevice(@Body request: DeviceCreateRequest): Response<DeviceDto>

    @POST("api/devices/{device_id}/share")
    suspend fun shareDevice(
        @Path("device_id") deviceId: String,
        @Body request: DeviceShareRequest
    ): Response<ApiResponse>

    @DELETE("api/devices/{device_id}/share/{target_user_id}")
    suspend fun revokeShare(
        @Path("device_id") deviceId: String,
        @Path("target_user_id") targetUserId: String
    ): Response<ApiResponse>

    @DELETE("api/devices/{device_id}")
    suspend fun deleteDevice(
        @Path("device_id") deviceId: String
    ): Response<ApiResponse>

    // ==================== PUMP CONTROL & TELEMETRY ====================
    @POST("api/pumps/{device_id}/control")
    suspend fun controlPump(
        @Path("device_id") deviceId: String,
        @Body request: PumpControlRequest
    ): Response<ApiResponse>

    @POST("api/pumps/{device_id}/config")
    suspend fun updateTankConfig(
        @Path("device_id") deviceId: String,
        @Body request: TankConfigRequest
    ): Response<ApiResponse>

    @GET("api/pumps/{device_id}/logs")
    suspend fun getPumpLogs(
        @Path("device_id") deviceId: String,
        @Query("limit") limit: Int = 50
    ): Response<PumpLogsResponse>

    // ==================== ADMIN ====================
    @GET("api/admin/users")
    suspend fun getAdminUsers(): Response<List<AdminUserDto>>

    @POST("api/admin/users")
    suspend fun createAdminUser(@Body request: AdminUserCreateRequest): Response<AdminUserDto>

    @PUT("api/admin/users/{user_id}")
    suspend fun updateAdminUser(
        @Path("user_id") userId: String,
        @Body request: AdminUserUpdateRequest
    ): Response<AdminUserDto>

    @DELETE("api/admin/users/{user_id}")
    suspend fun deleteAdminUser(@Path("user_id") userId: String): Response<ApiResponse>

    @POST("api/admin/ota/trigger")
    suspend fun triggerOta(@Body request: OtaTriggerRequest): Response<ApiResponse>

    @GET("api/admin/ota/progress")
    suspend fun getOtaProgress(): Response<OtaProgressResponse>
}

// Giữ lại AuthApiService alias để tương thích ngược
typealias AuthApiService = PumpApiService
