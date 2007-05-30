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
   
package ru.rpunet.webmops.web.admin;

import ru.rpunet.webmops.model.Distribution;
import ru.rpunet.webmops.dao.DistributionManager;

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidateNestedProperties;

public class AddDistributionActionBean implements ActionBean {

	private ActionBeanContext context;
	
	@ValidateNestedProperties({
		@Validate(field="name", required=true),
		@Validate(field="description", required=true)
	})
	private Distribution distr;
	
	public void setDistr(Distribution distr) {
		this.distr = distr;
	}
	
	public Distribution getDistr() {
		return this.distr;
	}
	
	public ActionBeanContext getContext() {
		return context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = arg0;
	}
	
	@DefaultHandler
	@DontValidate
	public Resolution index() {
		distr = new Distribution();
		return new ForwardResolution("/admin/AddEditDistribution.jsp");
	}
	
	public Resolution quickSave() {
	
		DistributionManager distDAO = new DistributionManager();
		distDAO.makePersistent(distr);
			
		return new RedirectResolution("/admin/Distributions.action");
	}

}
