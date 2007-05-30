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
 * Custom stylesheet tag
 * using:
 * <pre>
 * <m:stylesheet name="main" />
 * </pre>
 * loads $APP_HOME/styles/main.css
 * 
 * @author Andrew Diakin
 *
 */
public class StylesheetTag extends TagSupport {
	/**
	 * 
	 */
	private static final long serialVersionUID = -2569312261334983877L;
	private String name = null;
	
	public String getName() {
		return this.name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public int doStartTag() throws JspTagException {
		HttpServletRequest req = (HttpServletRequest) pageContext.getRequest();
		String path = req.getContextPath();
		
		try {
			JspWriter out = pageContext.getOut();
			out.print("<link rel=\"stylesheet\" ");
			out.print("type=\"text/css\" ");
			out.print("href=\"");
			out.print(path + "/CssResourcesLoader.action?style=" + name + ".css\" />");
			
		} catch (Exception e) {
			throw new JspTagException("Oops... this is not the way it must go...");
		}
		
		return SKIP_BODY;
	}
	
	public int doEndTag() {
		return EVAL_PAGE;
	}
}
