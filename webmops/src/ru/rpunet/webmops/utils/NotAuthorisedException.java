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

	public NotAuthorisedException() {
		super();
	}
	
	public NotAuthorisedException(String reason) {
		super(reason);
	}
	
}
