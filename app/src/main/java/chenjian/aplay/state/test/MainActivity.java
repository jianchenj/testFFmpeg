package chenjian.aplay.state.test;


public class MainActivity { /*extends ActionBarActivity implements
		View.OnClickListener, UpdateUIListener {

	private MyStateMachine machine = null;
	private Handler mHandler = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		Button btn = (Button) findViewById(R.id.btn_time_out);
		btn.setOnClickListener(this);
		btn = (Button) findViewById(R.id.btn_tired);
		btn.setOnClickListener(this);
		btn = (Button) findViewById(R.id.btn_wake_up);
		btn.setOnClickListener(this);
		machine = new MyStateMachine();
		machine.registerListener(this);
		mHandler = new MyHandler(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onClick(View arg0) {
		switch (arg0.getId()) {

		case R.id.btn_wake_up:
			machine.wakeup();
			break;
		case R.id.btn_time_out:
			machine.timeout();
			break;
		case R.id.btn_tired:
			machine.tired();
			break;
		}
	}

	private void updateText(String text) {
		TextView tx = (TextView) findViewById(R.id.tv);
		tx.setText(text);
	}

	static class MyHandler extends Handler {
		WeakReference<MainActivity> mActivity = null;

		public MyHandler(MainActivity act) {
			mActivity = new WeakReference<MainActivity>(act);
		}

		@Override
		public void handleMessage(Message msg) {
			MainActivity activity = mActivity.get();
			activity.updateText((String) msg.obj);
		}

	};

	@Override
	public void update(String tip) {
		mHandler.obtainMessage(0, tip).sendToTarget();
	}*/
}
