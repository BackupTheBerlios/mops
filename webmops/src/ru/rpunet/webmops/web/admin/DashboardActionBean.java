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

import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.controller.LifecycleStage;
import ru.rpunet.webmops.dao.DistributionManager;
import ru.rpunet.webmops.dao.MirrorManager;
import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Distribution;
import ru.rpunet.webmops.model.Mirror;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
public class DashboardActionBean extends WebmopsActionBean {
	
	private PersonManager personDAO;
	private MirrorManager mirrorDAO;
	private DistributionManager distDAO;
	
	private List<Person> users;
	private List<Mirror> mirrors;
	private List<Distribution> dists;
	
	public int getMirrorsCount() {
		return this.mirrors.size();
	}
	
	public int getUsersCount() {
		return this.users.size();
	}
	
	public int getDistsCount() {
		return this.dists.size();
	}
	
	public List<Distribution> getDists() {
		return dists;
	}

	public List<Mirror> getMirrors() {
		return mirrors;
	}

	public List<Person> getUsers() {
		return users;
	}

	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		personDAO = new PersonManager();
		mirrorDAO = new MirrorManager();
		distDAO = new DistributionManager();
		
		users = personDAO.findAllPersons();
		mirrors = mirrorDAO.findAll();
		dists = distDAO.findAll();
	}
	
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/admin/Dashboard.jsp");
	}
	
}
