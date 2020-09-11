package chenjian.aplay.palyview

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import java.lang.RuntimeException

/**
 * 实际控制播放的类
 * 上层播放控件和c层库交互
 */
class VideoPlayer(private val listenerManager : VideoView.PlayListenerManager) : SurfaceHolder.Callback {
    companion object {
        const val TAG = "VideoPlayer"

        init {
            System.loadLibrary("native-lib")
        }
    }

    private var mSurfaceHolder: SurfaceHolder? = null

    //播放源
    private var mSource: String? = null

    //播放监听列表
    //private val mPlayerListeners = HashMap<Int, PlayListener>().toMutableMap()

    /**
     * c层库方法
     */
    private external fun nativePrepare(url: String, listenerManager: VideoView.PlayListenerManager)
    private external fun nativeSetSurface(surface: Surface)
    private external fun nativeStart()
    private external fun nativeSeek(ms: Long) //seek毫秒.
    private external fun nativePause()
    private external fun nativeClose()
    private external fun nativeSurfaceChanged(width: Int, height: Int)

    private fun setSurface(surface: Surface) {
        Log.i(TAG, "<setSurfaceView>")
        nativeSetSurface(surface)
    }

    fun close() {
        Log.i(TAG, "<close>")
        nativeClose()
    }

    fun seek(milliseconds: Long) {
        Log.i(TAG, "<seek> milliseconds : $milliseconds")
        nativeSeek(milliseconds)
    }

    fun start() {
        Log.i(TAG, "<start>")
        nativeStart()
    }

    fun setDataSource(path: String) {
        Log.i(TAG, "<setDataSource> $path")
        mSource = path
    }

    fun prepare() {
        Log.i(TAG, "<prepare>")
        // TODO: 2020/9/1 StateMachine?
        if (mSource != null) {
            nativePrepare(mSource!!, listenerManager)
        } else {
            throw RuntimeException("please data source is not set before prepare")
        }
    }


    /**
     * 实现 SurfaceHolder.Callback
     */
    override fun surfaceCreated(holder: SurfaceHolder?) {

    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        if (holder == null) throw RuntimeException("surfaceChanged , but SurfaceHolder null")
        nativeSurfaceChanged(width, height)
        setSurface(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
        nativePause()
    }

    /**
     * 设置SurfaceView，
     */
    fun setSurfaceView(surfaceView: SurfaceView) {
        mSurfaceHolder?.removeCallback(this)
        mSurfaceHolder = surfaceView.holder
        mSurfaceHolder!!.addCallback(this)
    }


}