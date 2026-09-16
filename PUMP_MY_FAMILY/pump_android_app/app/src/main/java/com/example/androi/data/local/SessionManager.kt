package com.example.androi.data.local

import android.content.Context
import android.content.SharedPreferences
import com.example.androi.data.api.RetrofitClient
import com.example.androi.data.model.UserInfo
import com.google.gson.Gson

class SessionManager(context: Context) {
    private val prefs: SharedPreferences =
        context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE)
    private val gson = Gson()

    companion object {
        private const val PREF_NAME = "pump_my_family_session"
        private const val KEY_TOKEN = "jwt_token"
        private const val KEY_USER = "user_info"

        @Volatile
        private var instance: SessionManager? = null

        fun getInstance(context: Context): SessionManager {
            return instance ?: synchronized(this) {
                instance ?: SessionManager(context.applicationContext).also { instance = it }
            }
        }
    }

    init {
        // Tự động load token vào RetrofitClient khi khởi tạo
        val savedToken = getToken()
        if (!savedToken.isNullOrBlank()) {
            RetrofitClient.authToken = savedToken
        }
    }

    fun saveSession(token: String, user: UserInfo) {
        prefs.edit()
            .putString(KEY_TOKEN, token)
            .putString(KEY_USER, gson.toJson(user))
            .apply()
        RetrofitClient.authToken = token
    }

    fun getToken(): String? {
        return prefs.getString(KEY_TOKEN, null)
    }

    fun getUser(): UserInfo? {
        val userJson = prefs.getString(KEY_USER, null) ?: return null
        return try {
            gson.fromJson(userJson, UserInfo::class.java)
        } catch (e: Exception) {
            null
        }
    }

    fun isLoggedIn(): Boolean {
        return !getToken().isNullOrBlank()
    }

    fun clearSession() {
        prefs.edit().clear().apply()
        RetrofitClient.authToken = null
    }
}
