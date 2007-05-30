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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

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
		
		return resolution;
	}
	
	protected boolean isLoggedIn(WebmopsActionBeanContext ctx) {
		return true;		
	}
	
	protected boolean isPermitted(WebmopsActionBeanContext ctx) {
		return true;
	}
 
}
