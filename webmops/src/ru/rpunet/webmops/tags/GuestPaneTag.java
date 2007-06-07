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

package ru.rpunet.webmops.tags;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.tagext.TagSupport;

import ru.rpunet.webmops.utils.SecurityInterceptor;

import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.controller.StripesFilter;

/**
 * Show content to all not logged in users
 * <pre>
 * <m:guest>
 *  Login link
 * </m:guest>
 * 
 * @author Andrew Diakin
 *
 */
public class GuestPaneTag extends TagSupport {

	/**
	 * 
	 */
	private static final long serialVersionUID = 7489263072834097364L;

	public int doStartTag() throws JspException {
		
		ActionBeanContext ctx;
		
		try {
			ctx = StripesFilter.getConfiguration().getActionBeanContextFactory().getContextInstance(
					(HttpServletRequest) pageContext.getRequest(),
					(HttpServletResponse) pageContext.getResponse());
		} catch (ServletException e) {
			e.printStackTrace();
			throw new JspException("ActionBeanContext init failed.");
		}
		
		if ( !SecurityInterceptor.isLoggedIn(ctx) ) {
			return EVAL_BODY_INCLUDE;
		}
		
		return SKIP_BODY;
	}
	
}
