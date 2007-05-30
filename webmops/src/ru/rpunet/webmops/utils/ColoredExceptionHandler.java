/**
 * 
 */
package ru.rpunet.webmops.utils;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import net.sourceforge.stripes.config.Configuration;
import net.sourceforge.stripes.exception.ExceptionHandler;

/**
 * Pretty colored exception handler )
 * 
 * @author andrew
 *
 */
public class ColoredExceptionHandler implements ExceptionHandler {


	public void handle(Throwable throwable, HttpServletRequest request,
			HttpServletResponse response) throws ServletException{
		
		if (throwable instanceof ServletException) {
			throw (ServletException) throwable;
		}
		
	}

	
	public void init(Configuration arg0) throws Exception {
	}

}
