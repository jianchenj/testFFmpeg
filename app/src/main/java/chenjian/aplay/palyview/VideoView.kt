package chenjian.aplay.palyview

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceView

/**
 *  播放控件，写在布局中
 *  继承自SurfaceView
 *  内部管理一个 VideoPlayer
 */
class VideoView : SurfaceView {
    constructor(context: Context) : super(context)

    constructor(context: Context, attributeSet: AttributeSet) : super(context, attributeSet)

    constructor(context: Context, attributeSet: AttributeSet, defStyleAttr: Int) : super(
            context,
            attributeSet,
            defStyleAttr
    )


    private val mPlayer: VideoPlayer
    private var listenerManager : PlayListenerManager

    init {
        listenerManager = PlayListenerManager()
        mPlayer = VideoPlayer(listenerManager)
        mPlayer.setSurfaceView(this)
    }

    fun setDataSource(path: String) {
        mPlayer.setDataSource(path)
    }

    fun start() {
        mPlayer.start()
    }

    fun prepare() {
        mPlayer.prepare()
    }

    fun setDataSourceAndPrepare(path: String) {
        mPlayer.setDataSource(path)
        mPlayer.prepare();
    }

    fun addPlayListener(playListener: PlayListener) {
        listenerManager.addPlayListener(playListener)
    }

    fun removePlayListener(playListener: PlayListener) {
        listenerManager.removePlayListener(playListener)
    }

    fun clearListeners() {
        listenerManager.clearListeners()
    }

    inner class PlayListenerManager() {
        private val mPlayerListeners = HashMap<Int, PlayListener>().toMutableMap()

        /**
         * 管理播放监听
         */
        fun addPlayListener(playListener: PlayListener) {
            mPlayerListeners[playListener.hashCode()] = playListener
        }

        fun removePlayListener(playListener: PlayListener) {
            mPlayerListeners.remove(playListener.hashCode())
        }

        fun clearListeners() {
            mPlayerListeners.clear()
        }

        fun onPrepare() {
            for ((hashcode,listener) in mPlayerListeners){
                listener.onPrepare()
            }
        }

        fun onProgress(progress: Int) {
            for ((hashcode,listener) in mPlayerListeners){
                listener.onProgress(progress)
            }
        }

        fun onError(errorCode: Int) {
            for ((hashcode,listener) in mPlayerListeners){
                listener.onError(errorCode)
            }
        }
    }

}