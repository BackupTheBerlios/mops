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
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import net.sourceforge.stripes.validation.EmailTypeConverter;
import net.sourceforge.stripes.validation.LocalizableError;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidateNestedProperties;
import net.sourceforge.stripes.validation.ValidationErrors;
import net.sourceforge.stripes.validation.ValidationMethod;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Register.action")
public class RegisterActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(RegisterActionBean.class);
	
	/**
	 * new f*king user
	 */
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
	
	private PersonManager personDAO;
	
	public Person getUser() {
		return this.user;
	}
	
	public void setUser(Person user) {
		this.user = user;
	}
	
	
	@ValidationMethod
	public void validate(ValidationErrors errors) {
		if ( personDAO.findPerson(this.user.getLogin()) != null ) {
			errors.add("user.login", new LocalizableError("usernameTaken"));
		}
	}

	@DontValidate
	@DefaultHandler
	public Resolution index() {
		log.debug("ooooyyyyy!!!!! going to default resolution!!!!");
		return new ForwardResolution("/Register.jsp");
	}
	
	@HandlesEvent("registerUser")
	public Resolution register() {
		/* а вот это нам нах не нужно
		try {
			String encryptedPassword = PasswordServiceImpl.getInstance().encrypt(user.getPassword());
			user.setPassword(encryptedPassword);
		} catch (SystemUnavaliableException e) {
			// хуя!
			e.printStackTrace();
		}
		*/
		// и ниипет!
		user.setGroup("USER");
		personDAO = new PersonManager();
		
		log.info("Registering user " + user.getLogin());
		try {
			personDAO.makePersistent(user);
		} catch (SystemUnavaliableException e) {
			e.printStackTrace();
		}
		
		return new RedirectResolution(ru.rpunet.webmops.web.DefaultActionBean.class);
	}
}
