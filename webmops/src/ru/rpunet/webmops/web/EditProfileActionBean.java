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
import ru.rpunet.webmops.utils.SystemUnavaliableException;
import ru.rpunet.webmops.utils.WebmopsActionBean;
import net.sourceforge.stripes.action.Before;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.LocalizableMessage;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import net.sourceforge.stripes.controller.LifecycleStage;
import net.sourceforge.stripes.validation.EmailTypeConverter;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidateNestedProperties;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/EditProfile.action")
public class EditProfileActionBean extends WebmopsActionBean {

	private static Log log = LogFactory.getLog(EditProfileActionBean.class);
	
	@ValidateNestedProperties({
		@Validate(field="firstName", minlength=3, required=true),
		@Validate(field="lastName", minlength=3, required=true),
		@Validate(field="email", required=true, converter=EmailTypeConverter.class)
	})
	private Person user;
	
	private PersonManager personDAO;
	
	@Validate(minlength=6, maxlength=20)
	private String newPassword;
	
	@Validate(minlength=6, maxlength=20, expression="this == newPassword")
	private String confirmPassword;
	
	
	public String getNewPassword() {
		return this.newPassword;
	}
	
	public String getConfirmPassword() {
		return this.confirmPassword;
	}
	
	public void setNewPassword(String newPassword) {
		this.newPassword = newPassword;		
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
	
	@Before(LifecycleStage.BindingAndValidation)
	public void preload() {
		if (personDAO == null)
			personDAO = new PersonManager();
		
		log.debug("Preloading user objects");
		
		user = personDAO.findPersonById(getContext().getUser().getId());
	}
	
	@DontValidate
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/EditProfile.jsp");
	}
	
	@HandlesEvent("saveProfile")
	public Resolution saveProfile() {
		
		if ( newPassword != null && confirmPassword != null) {
			log.debug("Updating user password");
			user.setPassword(newPassword);
		} 
		
		
		log.debug("Saving user " + user.getLogin());
		try {
			personDAO.saveOrUpdate(user);
		} catch (SystemUnavaliableException e) {
			e.printStackTrace();
		}
		
		getContext().setUser(user);
		getContext().getMessages().add(new LocalizableMessage("/users/EditProfile.action.successMessage"));
		
				
		return new RedirectResolution(ru.rpunet.webmops.web.UserProfileActionBean.class);
	}
}
