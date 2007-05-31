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

import java.util.ArrayList;
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
		String path = ctx.getActionBeanContext().getRequest().getRequestURI();
		log.debug("Processing security filter for " + path + ".");
		
		Resolution resolution = ctx.proceed();
		
		if ( isLoggedIn(ctx.getActionBeanContext()) ) {
			log.debug("User logged.");
			if ( isPermitted((WebmopsActionBeanContext) ctx.getActionBeanContext(), ctx.getActionBean()) ) {
				return resolution;
			} else {
				return new RedirectResolution("/Unauthorized.jsp");
			}
		} else {
			log.debug("User not logged in.");
			if ( !path.contains("/users/Login.action") && !path.contains("CssResourcesLoader.action"))
				return new RedirectResolution("/users/Login.action");
			
			return resolution;			
		}
		
	}
	
	protected boolean isLoggedIn(ActionBeanContext ctx) {
		return ((WebmopsActionBeanContext) ctx).getUser() != null;		
	}
	
	protected boolean isPermitted(WebmopsActionBeanContext ctx, ActionBean bean) {
		log.debug("Checking permissions.");
		Person user = ctx.getUser();
		String path = ctx.getRequest().getRequestURI();
		
		if (user == null) {
			// anonymous user
			if ( AppConfigurator.getConfig().getAnonymousUrls().contains(path) )
				return true;
			else
				return false;	
		}
		
		if ( user.getGroup().equalsIgnoreCase("user") ) {
			// current user is in 'users' group
			if (AppConfigurator.getConfig().getUsersUrls().contains(path))
				return true;
			else
				return false;
		} else if ( user.getGroup().equalsIgnoreCase("moderator")) {
			if (AppConfigurator.getConfig().getModeratorsUrls().contains(path))
				return true;
			else
				return false;
		} else if ( user.getGroup().equalsIgnoreCase("administrator") ) {
			return true;
		}
		
		return false;
	}
 
}
