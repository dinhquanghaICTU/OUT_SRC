package com.example.androi.data.model

import com.google.gson.annotations.SerializedName

data class LoginRequest(val email: String, val password: String)

data class LoginResponse(
        @SerializedName("access_token") val accessToken: String,
        @SerializedName("token_type") val tokenType: String,
        @SerializedName("user") val user: UserInfo
)

data class UserInfo(
        val id: String,
        val email: String,
        @SerializedName("full_name") val fullName: String,
        val role: String // Ví dụ: "ADMIN", "USER"
)

data class RegisterRequest(
        val email: String,
        val password: String,
        @SerializedName("full_name") val fullName: String
)

data class RegisterResponse(
        val id: String,
        val email: String,
        @SerializedName("full_name") val fullName: String,
        val role: String
)
