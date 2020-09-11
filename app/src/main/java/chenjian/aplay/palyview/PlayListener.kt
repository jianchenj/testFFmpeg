package chenjian.aplay.palyview

interface PlayListener {
    fun onPrepare()

    fun onProgress(progress: Int)

    fun onError(errorCode : Int)
}