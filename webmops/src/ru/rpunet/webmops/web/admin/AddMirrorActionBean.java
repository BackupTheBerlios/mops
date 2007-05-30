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

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.dao.DistributionManager;
import ru.rpunet.webmops.dao.MirrorManager;
import ru.rpunet.webmops.model.Distribution;
import ru.rpunet.webmops.model.Mirror;

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.controller.LifecycleStage;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidateNestedProperties;

public class AddMirrorActionBean implements ActionBean {

	private static Log log = LogFactory.getLog(AddDistributionActionBean.class);
	
	private ActionBeanContext context;
	private List<Distribution> distributions;
	
	@ValidateNestedProperties({
		@Validate(field="name", required=true),
		@Validate(field="url", required=true)
	})
	private Mirror mirror;
	
	private Long distributionId;
	
	public Long getDistributionId() {
		return distributionId;
	}
	
	public void setDistributionId(Long arg) {
		this.distributionId = arg;
	}
	
	public List<Distribution> getDistributions() {
		return this.distributions;
	}
	
	public Mirror getMirror() {
		return this.mirror;
	}
	
	public void setMirror(Mirror mirror) {
		this.mirror = mirror;
	}
	
	public ActionBeanContext getContext() {
		return context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = arg0;
	}
	
	@SuppressWarnings("unchecked")
	@DontValidate
	@DefaultHandler
	public Resolution index() {
		log.debug("Processing index request.");
		mirror = new Mirror();
		
		return new ForwardResolution("/admin/AddEditMirror.jsp");
	}
	
	@SuppressWarnings("unchecked")
	public Resolution addMirror() {
		log.debug("Saving mirror.");
		
		DistributionManager distDAO = new DistributionManager();
		MirrorManager mirrorDAO = new MirrorManager();
		
		mirror.setDistrubution(distDAO.findById(distributionId));
		mirrorDAO.makePersistent(mirror);
		
		return new RedirectResolution("/admin/Mirrors.action");
	}
	
	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		log.debug("Preload domain objects.");
		DistributionManager distDAO = new DistributionManager();
		this.distributions = distDAO.findAll();
	}
	

}
