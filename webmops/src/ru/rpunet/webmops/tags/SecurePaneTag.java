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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

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
 * If user id passed
 * <pre>
 * <mops:secure userId="N">
 *  Some content
 * </mops:secure>
 * 
 * then only if current user have such id content will be shown
 * 
 * @author Andrew Diakin
 *
 */
public class SecurePaneTag extends TagSupport {

	/**
	 * 
	 */
	private static final long serialVersionUID = 2751468362400451836L;
	
	private static Log log = LogFactory.getLog(SecurePaneTag.class);
	
	/**
	 * list of groups which have access to content
	 * groups parted with ','
	 * @see ru.rpunet.webmops.model.PersonGroups
	 */
	private String roles;
	
	private Long userId;
	
	public Long getUserId() {
		return this.userId;
	}
	
	public void setUserId(Long userId) {
		this.userId = userId;
	}
	
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
		
	
		
		// Important! we must check this first		
		if ( roles == null || roles.length() < 1 ) {
			if ( SecurityInterceptor.isLoggedIn(ctx) ) {
				return EVAL_BODY_INCLUDE; // user logged in, allow this content
			} else {
				return SKIP_BODY;
			}
			
		}
		
		if ( userId != null ) {
			log.info("User id " + userId + " passed, validating.");
			// some user id passed, so we allow content only if current user have same id
			if ( SecurityInterceptor.validateUserById(userId, ctx) ) {
				// all ok
				return EVAL_BODY_INCLUDE;
			} else {
				return SKIP_BODY;
			}
		}
		
		log.debug("Checking for valid role");
		log.debug("Passed roles: " + roles);
		
		// get list of roles
		List<String> allRoles = Arrays.asList(roles.trim().split(","));
		log.debug("PASSED ARRAY: "  + allRoles.toString());
		String r[] = roles.trim().split(",");
		for (String s : r) {
			log.debug("Role passed: " + s);
		}
		log.debug("Roles passed to SecurityInterceptor size: " + allRoles.size());
		if ( SecurityInterceptor.isUserInRole(allRoles, ctx) ) {
			return EVAL_BODY_INCLUDE;
		}
		
		return SKIP_BODY;
	}
	
}
