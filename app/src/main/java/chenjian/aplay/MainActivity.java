package chenjian.aplay;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        PermissionUtil.verifyStoragePermissions(this);

        // Example of a call to a native method
        //TextView tv = findViewById(R.id.sample_text);
        //open("/sdcard/DCIM/Camera/test1.mp4", this);
        //tv.setText(stringFromJNI());
        //startActivity(new Intent(this.getApplicationContext(), TestActivity.class));
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native boolean open(String url, Object handle);


}
