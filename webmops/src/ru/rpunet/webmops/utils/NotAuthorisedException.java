/**
 * 
 */
package ru.rpunet.webmops.utils;

/**
 * @author andrew
 *
 */
public class NotAuthorisedException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 2140999798071356384L;

	private String reason;
	
	public NotAuthorisedException() {}
	
	public NotAuthorisedException(String reason) {
		this.reason = reason;
	}
	
	public String toString() {
		return reason;
	}
	
}
