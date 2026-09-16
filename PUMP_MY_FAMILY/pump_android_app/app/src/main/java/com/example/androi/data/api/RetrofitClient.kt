package com.example.androi.data.api

import java.util.concurrent.TimeUnit
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

object RetrofitClient {
    private const val BASE_URL = "http://180.93.113.40/"

    var authToken: String? = null

    private val loggingInterceptor =
            HttpLoggingInterceptor().apply { level = HttpLoggingInterceptor.Level.BODY }

    private val okHttpClient =
            OkHttpClient.Builder()
                    .addInterceptor(loggingInterceptor)
                    .addInterceptor { chain ->
                        val requestBuilder = chain.request().newBuilder()

                        authToken?.let { token ->
                            requestBuilder.addHeader("Authorization", "Bearer $token")
                        }
                        chain.proceed(requestBuilder.build())
                    }
                    .connectTimeout(15, TimeUnit.SECONDS)
                    .readTimeout(15, TimeUnit.SECONDS)
                    .build()

    val authApi: PumpApiService by lazy {
        Retrofit.Builder()
                .baseUrl(BASE_URL)
                .client(okHttpClient)
                .addConverterFactory(GsonConverterFactory.create())
                .build()
                .create(PumpApiService::class.java)
    }

    val api: PumpApiService get() = authApi
}
