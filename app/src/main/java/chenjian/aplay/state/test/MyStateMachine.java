package chenjian.aplay.state.test;

import android.os.Message;
import android.util.Log;

import chenjian.aplay.state.StateMachine;
import chenjian.aplay.state.State;

public class MyStateMachine extends StateMachine {

	private final static String TAG = MyStateMachine.class.getSimpleName();

	private final static int MSG_WAKE_UP = 1;
	private final static int MSG_TIME_OUT = 2;
	private final static int MSG_TIRED = 3;

	private UpdateUIListener listener = null;

	public MyStateMachine() {
		super(TAG);
		addState(mDefaulteState, null);
		addState(mSleepState, mDefaulteState);
		addState(mWorkState, mDefaulteState);
		addState(mPlayState, mDefaulteState);
		setInitialState(mSleepState);
		start();
	}

	public void registerListener(UpdateUIListener l) {
		this.listener = l;
	}

	private void notifyUI(String text) {
		if (listener != null) {
			listener.update(text);
		}
	}

	public void wakeup() {
		sendMessage(MSG_WAKE_UP);
	}

	public void timeout() {
		sendMessage(MSG_TIME_OUT);
	}

	public void tired() {
		sendMessage(MSG_TIRED);
	}

	private State mDefaulteState = new DefaultState();

	class DefaultState extends State {

		@Override
		public boolean processMessage(Message msg) {
			notifyUI("DefaultState: wrong command");
			return true;
		}
	}

	private State mSleepState = new SleepState();

	class SleepState extends State {
		@Override
		public void enter() {
			Log.d(TAG, "enter " + getName());
			notifyUI(getName());
		}

		@Override
		public boolean processMessage(Message msg) {
			switch (msg.what) {
			case MSG_WAKE_UP:
				transitionTo(mWorkState);
				break;
			default:
				return false;
			}
			return true;
		}
	}

	private State mWorkState = new WorkState();

	class WorkState extends State {
		@Override
		public void enter() {
			Log.d(TAG, "enter " + getName());
			notifyUI(getName());
		}

		@Override
		public boolean processMessage(Message msg) {
			switch (msg.what) {
			case MSG_TIME_OUT:
				transitionTo(mPlayState);
				break;
			default:
				return false;
			}
			return true;
		}
	}

	private State mPlayState = new PlayState();

	class PlayState extends State {
		@Override
		public void enter() {
			Log.d(TAG, "enter " + getName());
			notifyUI(getName());
		}

		@Override
		public boolean processMessage(Message msg) {
			switch (msg.what) {
			case MSG_TIRED:
				transitionTo(mSleepState);
				break;
			default:
				return false;
			}
			return true;
		}
	}
}
