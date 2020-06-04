package chenjian.aplay;

import android.content.Context;

import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

public class Xplay extends GLSurfaceView implements Runnable, SurfaceHolder.Callback {

    public Xplay(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        Open("/sdcard/DCIM/Camera/test2.mp4", getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder var1) {
        new Thread(this).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder var1, int var2, int var3, int var4) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder var1) {

    }

    public native void Open(String url, Object surface);
}
