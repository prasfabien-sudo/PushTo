package fr.dobsonpushto.mobile;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import com.getcapacitor.BridgeActivity;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends BridgeActivity {
    private static final int SYSTEM_BAR_COLOR = Color.rgb(8, 10, 16);

    @Override
    public void onCreate(Bundle savedInstanceState) {
        WindowCompat.setDecorFitsSystemWindows(getWindow(), true);
        super.onCreate(savedInstanceState);
        applySystemBarAppearance();
    }

    @Override
    public void onResume() {
        super.onResume();
        applySystemBarAppearance();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) applySystemBarAppearance();
    }

    private void applySystemBarAppearance() {
        getWindow().setStatusBarColor(SYSTEM_BAR_COLOR);
        getWindow().setNavigationBarColor(SYSTEM_BAR_COLOR);
        getWindow().getDecorView().setBackgroundColor(SYSTEM_BAR_COLOR);
        getWindow().setNavigationBarContrastEnforced(false);

        WindowInsetsControllerCompat controller =
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        controller.setAppearanceLightStatusBars(false);
        controller.setAppearanceLightNavigationBars(false);
        controller.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        int systemUiVisibility = getWindow().getDecorView().getSystemUiVisibility();
        systemUiVisibility &= ~(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR | View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR);
        getWindow().getDecorView().setSystemUiVisibility(systemUiVisibility);
        if (getSharedPreferences("dobson-system-bars", MODE_PRIVATE).getBoolean("night", false)) {
            controller.hide(WindowInsetsCompat.Type.statusBars());
        } else {
            controller.show(WindowInsetsCompat.Type.statusBars());
        }
    }
}
