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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspTagException;
import javax.servlet.jsp.JspWriter;
import javax.servlet.jsp.tagext.TagSupport;

/**
 * Custom javascript tag for include default app scripts
 * using:
 * <pre>
 * <m:javascript include="application" />
 * <pre>
 * includes js from $APP_HOME/js/application.js
 * 
 * @author Andrew Diakin
 *
 */
public class JavaScriptIncludeTag extends TagSupport {

	/**
	 * 
	 */
	private static final long serialVersionUID = -4561437121214737280L;
	private String include = null;

	public String getInclude() {
		return include;
	}

	public void setInclude(String include) {
		this.include = include;
	}
	
	public int doStartTag() throws JspTagException {
		HttpServletRequest req = (HttpServletRequest) pageContext.getRequest();
		String path = req.getContextPath();
		
		try {
			JspWriter out = pageContext.getOut();
			out.println("<script type=\"text/javascript\""); 
			out.print("src=\"" + path + "/js/" + include + ".js\"></script>");
		} catch (Exception e) {
			throw new JspTagException("OMG! I can't write js tag!");
		}
		
		return SKIP_BODY;
	}
	
	public int doEndTag() {
		return EVAL_PAGE;
	}
	
}
