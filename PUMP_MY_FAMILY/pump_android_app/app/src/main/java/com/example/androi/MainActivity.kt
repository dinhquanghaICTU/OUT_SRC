package com.example.androi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.Crossfade
import androidx.compose.animation.core.tween
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Surface
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import com.example.androi.data.local.SessionManager
import com.example.androi.ui.navigation.MainAppContainer
import com.example.androi.ui.screens.auth.AuthMode
import com.example.androi.ui.screens.auth.AuthScreen
import com.example.androi.ui.screens.splash.SplashScreen
import com.example.androi.ui.theme.AndroiTheme
import com.example.androi.ui.theme.ScreenBackground

enum class AppNavState {
    SPLASH,
    AUTH,
    MAIN
}

/*
 * Entry point chính của ứng dụng
 */
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        val sessionManager = SessionManager.getInstance(this)

        setContent {
            AndroiTheme {
                Surface(modifier = Modifier.fillMaxSize(), color = ScreenBackground) {
                    var currentNavState by remember { mutableStateOf(AppNavState.SPLASH) }

                    Crossfade(
                        targetState = currentNavState,
                        animationSpec = tween(durationMillis = 500),
                        label = "RootNavigation"
                    ) { state ->
                        when (state) {
                            AppNavState.SPLASH -> {
                                SplashScreen(
                                    onSplashComplete = {
                                        // Giống Web dashboard: Nếu đã có token đăng nhập thì vào thẳng MAIN
                                        currentNavState = if (sessionManager.isLoggedIn()) {
                                            AppNavState.MAIN
                                        } else {
                                            AppNavState.AUTH
                                        }
                                    }
                                )
                            }
                            AppNavState.AUTH -> {
                                AuthScreen(
                                    initialMode = AuthMode.LOGIN,
                                    onAuthSuccess = { currentNavState = AppNavState.MAIN }
                                )
                            }
                            AppNavState.MAIN -> {
                                MainAppContainer(
                                    onLogout = {
                                        sessionManager.clearSession()
                                        currentNavState = AppNavState.AUTH
                                    }
                                )
                            }
                        }
                    }
                }
            }
        }
    }
}
