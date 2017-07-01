package com.hideakin.lib.time;

public class TimeMachine {

	static {
		System.loadLibrary("TimeMachine");
	}

	/**
	 * Sets the current date to the given one.
	 *
	 * @param year
	 * @param month
	 * @param day
	 */
	public native void setDate(int year, int month, int day);

	/**
	 * Sets the time difference in milliseconds to the given one.
	 *
	 * @param deltaInMilliseconds
	 */
	public native void setDelta(long deltaInMilliseconds);

	/**
	 * Sets the current date back to today.
	 */
	public native void reset();

}
