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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.LocalizableMessage;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.controller.LifecycleStage;
import net.sourceforge.stripes.validation.EmailTypeConverter;
import net.sourceforge.stripes.validation.LocalizableError;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidateNestedProperties;
import net.sourceforge.stripes.validation.ValidationErrors;
import net.sourceforge.stripes.validation.ValidationMethod;
import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.SystemUnavaliableException;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
public class AddEditUserActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(AddEditUserActionBean.class);
	
	private PersonManager personDAO;
	
	private Long personId;
	
	@ValidateNestedProperties({
		@Validate(field="firstName", minlength=3, required=true),
		@Validate(field="lastName", minlength=3, required=true),
		@Validate(field="email", required=true, converter=EmailTypeConverter.class),
		@Validate(field="login", required=true, minlength=4, maxlength=15),
		@Validate(field="password", required=true, minlength=6, maxlength=20)
	})
	private Person user;
	
	@Validate(required=true, minlength=6, maxlength=20, expression="this == user.password")
	private String confirmPassword;
	
	public String getConfigrmPassword() {
		
		return this.confirmPassword;
	}
	
	public void setConfirmPassword(String confirmPassword) {
		this.confirmPassword = confirmPassword;
	}
	
	public Person getUser() {
		return this.user;
	}
	
	public void setUser(Person user) {
		this.user = user;
	}
	
	public Long getPersonId() {
		return this.personId;
	}
	
	public void setPersonId(Long personId) {
		this.personId = personId;
	}
	
	
	@ValidationMethod
	public void validate(ValidationErrors errors) {
		if ( user.getId() != null ) {
			if ( personDAO.findPerson(this.user.getLogin()) != null ) {
				errors.add("user.login", new LocalizableError("usernameTaken"));
			}
		}
	}
	
	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		log.debug("Loading Person DAO");
		personDAO = new PersonManager();
		
		log.info("Requested user id " + getContext().getRequest().getParameter("personId"));
		if ( getContext().getRequest().getParameter("personId") != null ) {
			user = personDAO.findPersonById(Long.parseLong(getContext().getRequest().getParameter("personId")));
			log.info("Edit user " + user.getLogin());
		} else {
			log.info("Creating new user");
		}
	}
	
	@DontValidate
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/admin/AddEditUser.jsp");
	}
	
	@HandlesEvent("saveUser")
	public Resolution saveUser() {
		try {
			personDAO.saveOrUpdate(user);
		} catch (SystemUnavaliableException e) {
			e.printStackTrace();
		}
		
		getContext().getMessages().add(
				new LocalizableMessage("/users/Register.action.successMessage",
										this.user.getFirstName(),
										this.user.getLogin())
			);
		
		return new RedirectResolution("/admin/Dashboard.action");
	}
	
}
