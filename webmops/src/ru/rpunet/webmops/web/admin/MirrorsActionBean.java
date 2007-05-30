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

import ru.rpunet.webmops.model.Mirror;
import ru.rpunet.webmops.dao.MirrorManager;

import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.Resolution;

public class MirrorsActionBean implements ActionBean {

	private ActionBeanContext context;
	
	private List<Mirror> mirrors;
	
	public int getMirrorsCount() {
		return mirrors.size();
	}
	
	public List<Mirror> getMirrors() {
		return mirrors;
	}
	
	public void setMirrors(List<Mirror> mirrors) {
		this.mirrors = mirrors;
	}
	
	public ActionBeanContext getContext() {
		return context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = arg0;
	}
	
	@SuppressWarnings("unchecked")
	@DefaultHandler
	public Resolution index() {
		MirrorManager mirrorDAO = new MirrorManager();
		mirrors = mirrorDAO.findAll();	
		
		return new ForwardResolution("/admin/ListMirrors.jsp");
	}

}
