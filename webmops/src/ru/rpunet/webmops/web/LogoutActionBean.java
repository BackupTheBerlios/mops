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

package ru.rpunet.webmops.web;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.utils.WebmopsActionBean;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Logout.action")
public class LogoutActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(LogoutActionBean.class);
	
	@DefaultHandler
	public Resolution index() {
		log.info("Invalidating user");
		getContext().logout();
		
		return new RedirectResolution("/Default.action");
	}
}
