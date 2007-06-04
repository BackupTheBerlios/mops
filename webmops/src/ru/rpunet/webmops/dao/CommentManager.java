/**
 * 
 */
package ru.rpunet.webmops.dao;

import java.util.List;

import org.hibernate.Session;

import ru.rpunet.webmops.model.Comment;
import ru.rpunet.webmops.utils.HibernateUtil;

/**
 * @author andrew
 *
 */
public class CommentManager {
	
	private Session session;
	
	public CommentManager() {
		session = HibernateUtil.getSessionFactory().getCurrentSession();
	}
	
	public Comment findById(Long id) {
		return (Comment) session.load(Comment.class, id);
	}
	
	@SuppressWarnings("unchecked")
	public List<Comment> findAll() {
		return (List<Comment>) session.createCriteria(Comment.class).list();
	}
	
	public List<Comment> findAllByPackageId(Long id) {
		return null;
	}
	
	public void saveOrUpdate(Comment comment) {
		session.saveOrUpdate(comment);
	}
	
	public void makePersistent(Comment comment) {
		saveOrUpdate(comment);
	}
}
