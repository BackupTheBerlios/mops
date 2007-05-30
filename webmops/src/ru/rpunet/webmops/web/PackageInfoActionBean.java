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

import ru.rpunet.webmops.dao.PackageManager;
import ru.rpunet.webmops.model.Package;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.controller.LifecycleStage;

/**
 * Show package information
 * 
 * @author Andrew Diakin
 */
public class PackageInfoActionBean implements ActionBean {

	private static Log log = LogFactory.getLog(PackageInfoActionBean.class);

	private ActionBeanContext context;
	
	private Long id;
	private Package pkg;

	public Package getPkg() {
		return this.pkg;
	}
	
	public void setId(Long id) {
		this.id = id;
	}
	
	public Long getId() {
		return id;
	}
	
	public ActionBeanContext getContext() {
		return context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = arg0;
	}
	
	public int getFilesCount() {
		return this.pkg.getFiles().size();
	}
	
	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		log.debug("preload called.");
	}
	
	@DefaultHandler
	public Resolution show() {
		PackageManager packageDAO = new PackageManager();
		pkg = packageDAO.findById(id);
		
		if (pkg == null)
			return new RedirectResolution("/PackageNotFound.jsp");
		
		return new ForwardResolution("/PackageInfo.jsp");
	}

}
