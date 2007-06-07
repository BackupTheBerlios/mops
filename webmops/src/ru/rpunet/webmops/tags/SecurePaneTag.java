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

import java.util.Arrays;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.tagext.TagSupport;

import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.controller.StripesFilter;

import ru.rpunet.webmops.utils.SecurityInterceptor;

/**
 * Tag for secure content
 * <pre>
 * <mops:secure roles="ADMINISTRATOR">
 * 	Some text for admin only
 * </mops:secure>
 * </pre>
 * 
 * if no roles defined then we show content for every logged user
 * 
 * @author Andrew Diakin
 *
 */
public class SecurePaneTag extends TagSupport {

	/**
	 * 
	 */
	private static final long serialVersionUID = 2751468362400451836L;
	
	/**
	 * list of groups which have access to content
	 * groups parted with ','
	 * @see ru.rpunet.webmops.model.PersonGroups
	 */
	private String roles;
	
	public void setRoles(String groups) {
		this.roles = groups;
	}
	
	public String getRoles() {
		return this.roles;
	}
	
	public SecurePaneTag() {
		super();
	}
	
	public int doStartTag() throws JspException {
	
		ActionBeanContext ctx = null;
		
		try {
			ctx = StripesFilter.getConfiguration().getActionBeanContextFactory().getContextInstance(
					(HttpServletRequest) pageContext.getRequest(),
					(HttpServletResponse) pageContext.getResponse());
		} catch (ServletException e) {
			e.printStackTrace();
			throw new JspException("Shit happens!");
		}
		
	
		
		if ( roles == null || roles.length() < 1 ) {
			if ( SecurityInterceptor.isLoggedIn(ctx) ) {
				return EVAL_BODY_INCLUDE;
			} else {
				return SKIP_BODY;
			}
			
		}
		
		// get list of roles
		List<String> allRoles = Arrays.asList(roles.trim().split(","));
		if ( SecurityInterceptor.isUserInRole(allRoles, ctx) ) {
			return EVAL_BODY_INCLUDE;
		}
		
		return SKIP_BODY;
	}
	
}
