package chenjian.aplay

import android.os.Bundle
import android.os.Handler
import android.util.Log
import android.view.WindowManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import chenjian.aplay.palyview.PlayListener

import chenjian.aplay.palyview.VideoView

class MainActivity : AppCompatActivity() {
    private var mVideoView: VideoView? = null
    val handler : Handler = Handler()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        PermissionUtil.verifyStoragePermissions(this)
        mVideoView = findViewById(R.id.video_view)

//        handler.postDelayed(Runnable {
//            var layoutParam = mVideoView!!.layoutParams
//            layoutParam.height = WindowManager.LayoutParams.MATCH_PARENT
//            layoutParam.width = WindowManager.LayoutParams.MATCH_PARENT;
//            mVideoView!!.layoutParams = layoutParam
//        }, 5000)
    }

    override fun onResume() {
        super.onResume()
        // TODO: 2020/9/1 路径需要可以选择
        mVideoView!!.setDataSource("/sdcard/DCIM/Camera/111.mp4")
        mVideoView!!.addPlayListener(object : PlayListener {
            override fun onPrepare() {
                Log.i("testFFmpeg", "******* java onPrepare() *******")
                mVideoView!!.start()
            }

            override fun onProgress(progress: Int) {
                Log.i("testFFmpeg", "******* java onProgress()  ******* $progress")
            }

            override fun onError(errorCode: Int) {
                Log.i("testFFmpeg", "******* java onError() ******* $errorCode")
            }
        })

        mVideoView!!.addPlayListener(object : PlayListener {
            override fun onPrepare() {
                Log.i("testFFmpeg", "******* 22222 java onPrepare() *******")
                mVideoView!!.start()
            }

            override fun onProgress(progress: Int) {
                Log.i("testFFmpeg", "******* 22222 java onProgress()  ******* $progress")
            }

            override fun onError(errorCode: Int) {
                Log.i("testFFmpeg", "******* 2222 java onError() ******* $errorCode")
            }
        })
        mVideoView!!.prepare()
    }
}