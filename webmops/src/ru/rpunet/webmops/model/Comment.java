/**
 * 
 */
package ru.rpunet.webmops.model;

import java.util.Date;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.Table;

import org.hibernate.validator.NotNull;

/**
 * Comment for uploaded packages
 * 
 * @author andrew
 *
 */
@Entity
@Table(name="comments")
public class Comment {
	
	@Id
	@GeneratedValue(strategy=GenerationType.IDENTITY)
	private Long id;
	
	@NotNull
	private String header;
	
	@NotNull
	private String message;
	
	@NotNull
	@ManyToOne
	@JoinColumn(name="user_id")
	private Person user;
	
	@NotNull
	private Long packageId;

	@NotNull
	@Column(name="dt_published")
	private Date datePublished;
	
	public Date getDatePublished() {
		return this.datePublished;
	}
	
	public void setDatePublished(Date datePublished) {
		this.datePublished = datePublished;
	}
	
	public String getHeader() {
		return header;
	}

	public void setHeader(String header) {
		this.header = header;
	}

	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
	}

	public String getMessage() {
		return message;
	}

	public void setMessage(String message) {
		this.message = message;
	}

	public Long getPackageId() {
		return packageId;
	}

	public void setPackageId(Long packageId) {
		this.packageId = packageId;
	}

	public Person getUser() {
		return user;
	}

	public void setUser(Person user) {
		this.user = user;
	}
	
	
}
