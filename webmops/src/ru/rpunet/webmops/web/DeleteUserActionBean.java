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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.WebmopsActionBean;
import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import net.sourceforge.stripes.controller.LifecycleStage;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Delete.action")
public class DeleteUserActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(DeleteUserActionBean.class);
	
	private Long personId;
	private PersonManager personDAO;
	
	private Person user;
	
	public Person getUser() {
		return this.user;
	}
	
	public void setPersonId(Long id) {
		this.personId = id;
	}
	
	public Long getPersonId() {
		return this.personId;
	}
	
	
	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		if ( personDAO == null )
			personDAO = new PersonManager();
		
		log.debug("Getting user with id " + getContext().getRequest().getParameter("personId"));
		
		user = personDAO.findPersonById(Long.parseLong(getContext().getRequest().getParameter("personId")));
	}
	
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/admin/DeleteUser.jsp");
	}
	
	@HandlesEvent("deleteUser") 
	public Resolution deleteUser() {
		
		personDAO.deletePerson(personId);
		
		return new RedirectResolution(ru.rpunet.webmops.web.admin.DashboardActionBean.class);
	}
}

