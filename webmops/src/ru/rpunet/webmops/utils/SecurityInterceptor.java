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

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.model.Person;

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.controller.ExecutionContext;
import net.sourceforge.stripes.controller.Interceptor;
import net.sourceforge.stripes.controller.Intercepts;
import net.sourceforge.stripes.controller.LifecycleStage;

/**
 * Security filter around all requests
 * 
 * @author Andrew Diakin
 *
 */
@Intercepts(LifecycleStage.HandlerResolution)
public class SecurityInterceptor implements Interceptor {

	private static Log log = LogFactory.getLog(SecurityInterceptor.class);
	
	public Resolution intercept(ExecutionContext ctx) throws Exception {
		
		if ( AppConfigurator.isConfigured() ) {
			log.info("Application configured.");
		}
		
		String path = ctx.getActionBeanContext().getRequest().getRequestURI();
		path = path.replace("/" + ctx.getActionBeanContext().getServletContext().getServletContextName(), "");
		log.debug("Processing security filter for " + path + ".");
	
		
		Resolution resolution = ctx.proceed();
		
		try {
			if ( isPermitted((WebmopsActionBeanContext) ctx.getActionBeanContext(), ctx.getActionBean()) ) {
				return resolution;
			}
		} catch (NotAuthenticatedException ex) {
			return new RedirectResolution("/users/Login.action?targetUrl=" + path);
		} catch (NotAuthorisedException ex) {
			return new RedirectResolution("/PermissionDenied.jsp");
		} finally {
			// if everithing else failed... 
			resolution = new RedirectResolution("/ApplicationError.jsp");
		}
		
		return resolution;
	}
	
	
	protected boolean isPermitted(WebmopsActionBeanContext ctx, ActionBean bean) 
			throws NotAuthenticatedException, NotAuthorisedException {
		Person user = ctx.getUser();
		String path = ctx.getRequest().getRequestURI();
		String contextPath = ctx.getServletContext().getServletContextName();
		// remove contextPath from requested url
		path = path.replace("/" + contextPath, "");
		log.debug("Checking permissions for user " + user + " (" + path + ") (" + contextPath + ")");
		
		if (user == null) {
			// anonymous user
			log.debug("user anonymous");
			if ( AppConfigurator.getConfig().getAnonymousUrls().contains(path) )
				return true;
			else
				throw new NotAuthenticatedException("Anonymous user access denied");	
		} else if ( user.getGroup().equalsIgnoreCase("user") ) {
			// current user is in 'users' group
			if (AppConfigurator.getConfig().getUsersUrls().contains(path))
				return true;
			else
				throw new NotAuthorisedException("User access denied");
			
		} else if ( user.getGroup().equalsIgnoreCase("moderator")) {
			if (AppConfigurator.getConfig().getModeratorsUrls().contains(path))
				return true;
			else
				throw new NotAuthorisedException("Access denied");
			
		} else if ( user.getGroup().equalsIgnoreCase("administrator") ) {
			return true;
		}
		
		return false;
	}
	
	/**
	 * check if current user have same id as requested
	 * @param id user id
	 * @param ctx Current ActionBean context
	 * @return result
	 */
	public static boolean validateUserById(Long id, ActionBeanContext ctx) {
		Person user = ((WebmopsActionBeanContext) ctx).getUser();
		
		if ( user!= null && user.getId() == id ) {
			// user logged in and id equals
			return true;
		} else {
			return false;
		}
	}
	
	public static boolean isUserInRole(List<String> roles, ActionBeanContext ctx) {
		
		Person user = ((WebmopsActionBeanContext) ctx).getUser();

		String _roles = "";
		
		for (String _r : roles) {
			_roles += _r + ", ";
		}
		
		if ( user == null ) {
			log.info("User is in ANONYMOUS group");
			return false;
		}
		
		if ( user.getGroup().equalsIgnoreCase("ADMINISTRATOR") ) {
			log.info("User admin, everything allowed");
			return true;
		}
		
		log.info("Checking if user " + user.getLogin() + " [" + user.getGroup() + "] " + " in roles(" + _roles + ")");;
		
		
		for (String role : roles) {
			if ( user.getGroup().equalsIgnoreCase(role) ) {
				log.info("User in role " + role);
				return true;
			}
		}
		
		return false;
	}
	
	public static boolean isLoggedIn(ActionBeanContext ctx) {
		Person user = ((WebmopsActionBeanContext) ctx).getUser();
		
		if ( user == null ) {
			log.info("User not logged in.");
			return false;
		}
		
		log.info("User logged in.");
		return true;
	}
	
 
}
