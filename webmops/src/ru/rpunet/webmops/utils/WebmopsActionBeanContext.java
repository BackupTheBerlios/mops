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

import ru.rpunet.webmops.model.Person;
import net.sourceforge.stripes.action.ActionBeanContext;

/**
 * Custom context implementation
 * 
 * usage: add to web.xml
 * <pre>
 * <init-param>
 * 	<param-name>ActionBeanContext.Class</param-name>
 * 	<param-value>ru.rpunet.webmops.utils.WebmopsActionBeanContext</param-value>
 * </init-param>
 * </pre>
 * and reload application
 * Important: not all action beans using this context now
 * 
 * @author Andrew Diakin
 *
 */
public class WebmopsActionBeanContext extends ActionBeanContext {
	
	/**
	 * get currently logged in user or null
	 * @return
	 */
	public Person getUser() {
		return (Person) getRequest().getSession().getAttribute("user");
	}
	
	/**
	 * set currently logged in user
	 * @param user
	 */
	public void setUser(Person user) {
		getRequest().getSession().setAttribute("user", user);
	}
	
	/**
	 * logout by invalidating session
	 *
	 */
	public void logout() {
		getRequest().getSession().invalidate();
	}
	
}
