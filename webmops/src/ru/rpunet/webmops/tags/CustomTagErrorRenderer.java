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

import java.io.IOException;

import net.sourceforge.stripes.tag.InputTagSupport;
import net.sourceforge.stripes.tag.TagErrorRenderer;

/**
 * Custom error tag renderer. Add asterisk before input tag.
 * 
 * To use it add to Stripes Filter config in web.xml:
 * <init-param>
 *  <param-name>TagErrorRenderer.Class</param-name>
 *  <param-value>ru.rpunet.webmops.tags.CustomTagErrorRenderer</param-value>
 * </init-param>
 * 
 * @author Andrew Diakin
 *
 */
public class CustomTagErrorRenderer implements TagErrorRenderer {

	private InputTagSupport tag;
	private String oldCssClass;

	public void doAfterEndTag() {
		tag.setCssClass(oldCssClass);
	}
	
	/**
     * Returns the tag which is being rendered. Useful mostly when subclassing the default
     * renderer to add further functionality.
     *
     * @return the input tag being rendered
     */
    protected InputTagSupport getTag() {
        return this.tag;
    }


	public void doBeforeStartTag() {
        try {
            this.tag.getPageContext().getOut().write
                ("<span style=\"color: red; font-weight: bold;\">*</span>");
            
            this.oldCssClass = tag.getCssClass();
            if (this.oldCssClass != null && this.oldCssClass.length() > 0) {
                tag.setCssClass("error " + this.oldCssClass);
            }
            else {
                tag.setCssClass("error");
            }
        } catch (IOException ex) {
            // Oops...
        }
	}


	public void init(InputTagSupport arg0) {
		this.tag = arg0;
	}

}
