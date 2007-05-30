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

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;

public abstract class AbstractAction implements ActionBean {

	private WebmopsActionBeanContext context = null;
	
	public WebmopsActionBeanContext getContext() {
		return this.context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = (WebmopsActionBeanContext) arg0;
		
		preBind();
	}
	
	protected void preBind() {
		
	}

}
