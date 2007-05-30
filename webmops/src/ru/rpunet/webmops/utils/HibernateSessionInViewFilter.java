/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
   
package ru.rpunet.webmops.utils;

import java.io.IOException;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.SessionFactory;
import org.hibernate.StaleObjectStateException;

/**
 * Open Session in View pattern implementation
 * 
 * @author Andrew Diakin
 *
 */
public class HibernateSessionInViewFilter implements Filter {

	private static Log log = LogFactory.getLog(HibernateSessionInViewFilter.class);
	
	private SessionFactory sf;
	
	public void destroy() {
	}

	public void doFilter(ServletRequest request, ServletResponse response,
			FilterChain chain) throws IOException, ServletException {

		try {
			log.debug("Starting database transaction.");
			
			// start transaction
			sf.getCurrentSession().beginTransaction();
			
			// call to next filter
			chain.doFilter(request, response);
			
			// commit transaction
			sf.getCurrentSession().getTransaction().commit();
		} catch (StaleObjectStateException e) {
            log.error("This interceptor does not implement optimistic concurrency control!");
            log.error("Your application will not work until you add compensation actions!");
		} catch (Throwable ex) {
			// rollback if some exception occured
			ex.printStackTrace();
			try {
				if ( sf.getCurrentSession().getTransaction().isActive() ) {
					log.debug("Trying to rollback transaction after exception");
					sf.getCurrentSession().getTransaction().rollback();
				}
			} catch (Throwable	rEx) {
				log.error("Cannot rollback transaction after exception!", rEx);
			}
			
			throw new ServletException(ex);
		}
	}
	

	public void init(FilterConfig filterConfig) throws ServletException {
		log.debug("Initializing filter...");
		sf = HibernateUtil.getSessionFactory();
	}

}
